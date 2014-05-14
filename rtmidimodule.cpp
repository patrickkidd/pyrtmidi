/*
 #   Copyright (C) 2011 by Patrick Stinson
 #   patrickkidd@gmail.com
 #
 #   This program is free software; you can redistribute it and/or modify
 #   it under the terms of the GNU General Public License as published by
 #   the Free Software Foundation; either version 2 of the License, or
 #   (at your option) any later version.
 #
 #   This program is distributed in the hope that it will be useful,
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #   GNU General Public License for more details.
 #
 #   You should have received a copy of the GNU General Public License
 #   along with this program; if not, write to the
 #   Free Software Foundation, Inc.,
 #   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "PyMidiMessage.h"
#include <RtMidi.h>
#include <Python.h>
#include <queue>

#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE Py_INCREF(Py_None); return Py_None;
#endif

#ifdef __APPLE__
#define PK_WINDOWS 0
#endif
#ifdef __LINUX__
#define PK_WINDOWS 0
#endif
#ifdef __WINDOWS__
#define PK_WINDOWS 1
#endif

#if PY_MAJOR_VERSION == 3
#define PK_PYTHON3 1
#else
#define PK_PYTHON3 0
#endif



static PyObject *PyRtMidiError;

typedef struct
{
  PyObject_HEAD

  RtMidiIn *rtmidi;
  PyObject *callback;
  long calling_thread_id;
  std::queue<MidiMessage *> *m_q;

#if PK_WINDOWS
  HANDLE mutex;
  HANDLE cond;
#else
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool triggered;
#endif

} MidiIn;



static void MidiIn_callback(double timestamp, std::vector<unsigned char> *message, void *opaque)
{
  MidiIn *self = (MidiIn *) opaque;

  uint8 *data = (uint8*) malloc(message->size());
  for(size_t i=0; i < message->size(); i++)
    data[i] = (*message)[i];
  MidiMessage inMidi(data, (int) message->size(), timestamp);
  free(data);

#if PK_WINDOWS
  WaitForSingleObject(self->mutex);
#else
  pthread_mutex_lock(&self->mutex);
#endif

  if(self->callback) // called too fast to init?
  {
    PyGILState_STATE gil_state = PyGILState_Ensure();

    PyObject *result = NULL;
    PyObject *args = NULL;

    PyMidiMessage *outMidi = (PyMidiMessage *) PyMidiMessage_new();

    (*outMidi->m) = inMidi;

    args = Py_BuildValue("(O)", outMidi);
    result = PyEval_CallObject(self->callback, args);
    if(result == NULL)
    {
      PyThreadState_SetAsyncExc(self->calling_thread_id, 0);
    }

    Py_XDECREF(result);
    Py_XDECREF(args);
    Py_XDECREF(outMidi);

    PyGILState_Release(gil_state);
  }
  else
  {
    self->m_q->push(new MidiMessage(inMidi));
    self->triggered = true;
  }

#if PK_WINDOWS
  SetEvent(self->cond);
  ReleaseMutex(self->mutex);
#else
  pthread_cond_signal(&self->cond);
  pthread_mutex_unlock(&self->mutex);
#endif
}


static PyObject *
MidiIn_getMessage(MidiIn *self, PyObject *args)
{
  PyObject *timeout = NULL;
  PyObject *result = NULL;

  if(!PyArg_ParseTuple(args, "|O:getMessage", &timeout))
    return NULL;

  long ms = -1;
  if(timeout)
  {
    if(PyFloat_CheckExact(timeout))
      ms = (long) PyFloat_AS_DOUBLE(timeout);
#if ! PK_PYTHON3
    else if(PyInt_CheckExact(timeout))
      ms = PyInt_AS_LONG(timeout);
#endif
    else if(PyLong_CheckExact(timeout))
      ms = PyLong_AsLong(timeout);
    else
    {
      PyErr_Format(PyRtMidiError, "timeout value must be a number, not %s", timeout->ob_type->tp_name);
      return NULL;
    }

    if(ms < 0)
    {
      PyErr_SetString(PyRtMidiError, "timeout value must be a positive number");
      return NULL;
    }
  }

#if PK_WINDOWS
  WaitForSingleObject(self->mutex);
#else
  pthread_mutex_lock(&self->mutex);
#endif

  if(ms > -1 && self->triggered == false)
  {
    PyThreadState *_save = PyEval_SaveThread();

#if PK_WINDOWS
    WaitForSingleObject(self->cond, ms < 0 ? INFINITE : ms);
#else
    if(ms < 0)
    {
      pthread_cond_wait(&self->cond, &self->mutex);
    }
    else
    {
      struct timeval now;
      gettimeofday(&now, 0);

      struct timespec time;
      TIMEVAL_TO_TIMESPEC(&now, &time);
      unsigned long sec = ms / 1000;
      unsigned long nsec = (ms % 1000) * 1000000;

      time.tv_sec += sec;
      time.tv_nsec += nsec;
      while(time.tv_nsec >= 1000000000) {
        time.tv_sec++;
        time.tv_nsec -= 1000000000;
      }
      pthread_cond_timedwait(&self->cond, &self->mutex, &time);
    }
#endif

    PyEval_RestoreThread(_save);
  }

  if(self->m_q->size() > 0)
  {
    result = (PyObject *) PyMidiMessage_new();
    MidiMessage *inMidi = self->m_q->front();
    (*((PyMidiMessage *)result)->m) = (*inMidi);
    self->m_q->pop();
    self->triggered = false;
    delete inMidi;
  }

#if PK_WINDOWS
  ReleaseMutex(self->mutex);
#else
  pthread_mutex_unlock(&self->mutex);
#endif

  if(result)
    return result;

  Py_RETURN_NONE;
}

static PyObject *
MidiIn_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "|s", &name))
    return NULL;

  MidiIn *self;
  self = (MidiIn *) type->tp_alloc(type, 0);

  if(self != NULL)
  {
    if (name == NULL)
    {
      try
      {
        self->rtmidi = new RtMidiIn();
      }
      catch(RtMidiError &error)
      {
        PyErr_SetString(PyRtMidiError, error.getMessageString());
        Py_DECREF(self);
        return NULL;
      }
    }
    else
    {
      try
      {
        self->rtmidi = new RtMidiIn();
      }
      catch(RtMidiError &error)
      {
        PyErr_SetString(PyRtMidiError, error.getMessageString());
        Py_DECREF(self);
        return NULL;
      }
    }
  }

#if PK_WINDOWS
  self->mutex = CreateMutex (0, FALSE, 0);
  self->cond = CreateEvent (0, FALSE, FALSE, 0);
#else
  pthread_cond_init (&self->cond, 0);
  pthread_mutex_init (&self->mutex, 0);
#endif

  self->m_q = new std::queue<MidiMessage *>;
  self->callback = 0;
  self->calling_thread_id = -1;
  return (PyObject *) self;
}

static void
MidiIn_dealloc(MidiIn *self)
{
#if PK_WINDOWS
  WaitForSingleObject(self->mutex, INFINITE);
#else
  pthread_mutex_lock(&self->mutex);
#endif

  self->rtmidi->closePort();
  delete self->rtmidi;
  delete self->m_q;
  self->m_q = NULL;
  Py_XDECREF(self->callback);
  self->callback = NULL;
  self->ob_type->tp_free((PyObject *) self);

#if PK_WINDOWS
  ReleaseMutex(self->mutex);
  CloseHandle (self->mutex);
  CloseHandle (self->cond);
#else
  pthread_mutex_unlock(&self->mutex);
  pthread_mutex_destroy(&self->mutex);
  pthread_cond_destroy(&self->cond);
#endif

}

static int
MidiIn_init(MidiIn *self, PyObject *args, PyObject *kwds)
{
  return 0;
}


static PyObject *
MidiIn_openPort(MidiIn *self, PyObject *args)
{
  int port;
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "i|s", &port, &name))
    return NULL;

  if (name == NULL)
  {
    try
    {
      self->rtmidi->openPort(port);
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }

  else
  {
    try
    {
      self->rtmidi->openPort(port,name);
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }

  self->rtmidi->setCallback(MidiIn_callback, self);
  self->calling_thread_id = PyThreadState_Get()->thread_id;

  Py_RETURN_NONE;
}


static PyObject *
MidiIn_openVirtualPort(MidiIn *self, PyObject *args)
{
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "|s", &name))
    return NULL;

  if(name == NULL)
  {
    try
    {
      self->rtmidi->openVirtualPort();
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }
  else
  {
    try
    {
      self->rtmidi->openVirtualPort(name);
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }

  self->rtmidi->setCallback(MidiIn_callback, self);
  self->calling_thread_id = PyThreadState_Get()->thread_id;

  Py_RETURN_NONE;
}




static PyObject *
MidiIn_setCallback(MidiIn *self, PyObject *args)
{
  PyObject *callback = NULL;

  if(!PyArg_ParseTuple(args, "O:setCallback", &callback))
    return NULL;

  if(!PyCallable_Check(callback))
  {
    PyErr_SetString(PyExc_TypeError, "parameter must be a callable object");
    return NULL;
  }

  // delete the old one if set
  Py_XDECREF(self->callback);

  // install the new one
  self->callback = callback;
  Py_INCREF(self->callback);

  Py_RETURN_NONE;
}



static PyObject *
MidiIn_cancelCallback(MidiIn *self, PyObject *)
{
  self->rtmidi->cancelCallback();
  if(self->callback)
  {
    Py_DECREF(self->callback);
    self->callback = NULL;
  }

  Py_RETURN_NONE;
}


static PyObject *
MidiIn_closePort(MidiIn *self, PyObject *)
{
  self->rtmidi->closePort();

  Py_RETURN_NONE;
}


static PyObject *
MidiIn_getPortCount(MidiIn *self, PyObject *)
{
  return PyInt_FromLong(self->rtmidi->getPortCount());
}


static PyObject *
MidiIn_getPortName(MidiIn *self, PyObject *args)
{
  int port;
  std::string name;

  if(!PyArg_ParseTuple(args, "i", &port))
    return NULL;

  try
  {
    name = self->rtmidi->getPortName(port);
  }
  catch(RtMidiError &error)
  {
    PyErr_SetString(PyRtMidiError, error.getMessageString());
    return NULL;
  }

  return Py_BuildValue("s", name.c_str());
}


static PyObject *
MidiIn_ignoreTypes(MidiIn *self, PyObject *args)
{
  PyObject *omidiSysex = Py_True;
  PyObject *omidiTime = Py_True;
  PyObject *omidiSense = Py_True;

  bool midiSysex;
  bool midiTime;
  bool midiSense;

  if(!PyArg_ParseTuple(args, "|OOO", &omidiSysex, &omidiTime, &omidiSense))
    return NULL;

  midiSysex = PyObject_IsTrue(omidiSysex);
  midiTime = PyObject_IsTrue(omidiTime);
  midiSense = PyObject_IsTrue(omidiSense);

  self->rtmidi->ignoreTypes(midiSysex, midiTime, midiSense);

  Py_RETURN_NONE;
}


static PyMethodDef MidiIn_methods[] = {
  {"openPort", (PyCFunction) MidiIn_openPort, METH_VARARGS,
    "Open a MIDI input connection. openPort(port, blocking=False)\n"
    "If the optional blocking parameter is passed getMessage will block until a message is available."},

  {"openVirtualPort", (PyCFunction) MidiIn_openVirtualPort, METH_VARARGS,
    "Create a virtual input port, with optional name, to allow software "
    "connections (OS X and ALSA only)."},

  {"setCallback", (PyCFunction) MidiIn_setCallback, METH_VARARGS,
    "Set a callback function to be invoked for incoming MIDI messages."},

  {"cancelCallback", (PyCFunction) MidiIn_cancelCallback, METH_NOARGS,
    "Cancel use of the current callback function (if one exists)."},

  {"closePort", (PyCFunction) MidiIn_closePort, METH_NOARGS,
    "Close an open MIDI connection (if one exists)."},

  {"getPortCount", (PyCFunction) MidiIn_getPortCount, METH_NOARGS,
    "Return the number of available MIDI input ports."},

  {"getPortName", (PyCFunction) MidiIn_getPortName, METH_VARARGS,
    "Return a string identifier for the specified MIDI input port number."},

  {"ignoreTypes", (PyCFunction) MidiIn_ignoreTypes, METH_VARARGS,
    "Specify whether certain MIDI message types should be queued or ignored "
    "during input."},

  {"getMessage", (PyCFunction) MidiIn_getMessage, METH_VARARGS,
    "Return the data bytes for the next available MIDI message in"
    "the input queue and return the event delta-time in seconds.\n"
    "This method will block if openPort() was called in blocking mode"},

  {NULL}
};


static PyTypeObject MidiIn_type = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "midi.RtMidiIn",             /*tp_name*/
  sizeof(MidiIn), /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor) MidiIn_dealloc,                         /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
  "Midi input device",           /* tp_doc */
  0,               /* tp_traverse */
  0,               /* tp_clear */
  0,               /* tp_richcompare */
  0,               /* tp_weaklistoffset */
  0,               /* tp_iter */
  0,               /* tp_iternext */
  MidiIn_methods,             /* tp_methods */
  0,              /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)MidiIn_init,      /* tp_init */
  0,                         /* tp_alloc */
  MidiIn_new,                 /* tp_new */
};






typedef struct {
  PyObject_HEAD
  /* Type-specific fields go here. */
  RtMidiOut *rtmidi;
} MidiOut;


static void
MidiOut_dealloc(MidiOut *self)
{
  delete self->rtmidi;
  self->ob_type->tp_free((PyObject *) self);
}


static PyObject *
MidiOut_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "|s", &name))
    return NULL;

  MidiOut *self;
  self = (MidiOut *) type->tp_alloc(type, 0);
  if(self != NULL)
  {
    if (name == NULL)
    {
      try
      {
        self->rtmidi = new RtMidiOut;
      }
      catch(RtMidiError &error)
      {
        PyErr_SetString(PyRtMidiError, error.getMessageString());
        Py_DECREF(self);
        return NULL;
      }
    }
    else
    {
      try
      {
        self->rtmidi = new RtMidiOut();
      }
      catch(RtMidiError &error)
      {
        PyErr_SetString(PyRtMidiError, error.getMessageString());
        Py_DECREF(self);
        return NULL;
      }
    }
  }
  return (PyObject *) self;
}

static int
MidiOut_init(MidiOut *self, PyObject *args, PyObject *kwds)
{
  return 0;
}


static PyObject *
MidiOut_openPort(MidiOut *self, PyObject *args)
{
  int port = 0;
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "i|s", &port, &name))
    return NULL;

  if (name == NULL)
    try
    {
      self->rtmidi->openPort(port);
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  else
  {
    try
    {
      self->rtmidi->openPort(port,name);
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
MidiOut_openVirtualPort(MidiOut *self, PyObject *args)
{
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "|s", &name))
    return NULL;

  if(name == NULL)
  {
    try
    {
      self->rtmidi->openVirtualPort();
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }
  else
  {
    try
    {
      self->rtmidi->openVirtualPort(name);
    }
    catch(RtMidiError &error)
    {
      PyErr_SetString(PyRtMidiError, error.getMessageString());
      return NULL;
    }
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
MidiOut_closePort(MidiOut *self, PyObject *args)
{
  self->rtmidi->closePort();
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
MidiOut_getPortCount(MidiOut *self, PyObject *args)
{
  return Py_BuildValue("i", self->rtmidi->getPortCount());
}


static PyObject *
MidiOut_getPortName(MidiOut *self, PyObject *args)
{
  int port;
  std::string name;

  if(!PyArg_ParseTuple(args, "i", &port))
    return NULL;

  try
  {
    name = self->rtmidi->getPortName(port);
  }
  catch(RtMidiError &error)
  {
    PyErr_SetString(PyRtMidiError, error->getMessage());
    return NULL;
  }

  return Py_BuildValue("s", name.c_str());
}


static PyObject *
MidiOut_sendMessage(MidiOut *self, PyObject *args)
{

  PyObject *a0 = NULL;
  if(PyArg_ParseTuple(args, "O", &a0) == 0)
    return NULL;

  if(!PyMidiMessage_Check(a0))
  {
    PyErr_SetString(PyRtMidiError, "argument 1 must be of type MidiMessage");
    return NULL;
  }

  PyMidiMessage *midi = (PyMidiMessage *) a0;

  std::vector<unsigned char> outMessage;
  uint8 *data = midi->m->getRawData();
  for(int i=0; i < midi->m->getRawDataSize(); ++i)
    outMessage.push_back((unsigned char) data[i]);

  try
  {
    self->rtmidi->sendMessage(&outMessage);
  }
  catch(RtMidiError &error)
  {
    PyErr_SetString(PyRtMidiError, error->getMessage());
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyMethodDef MidiOut_methods[] = {
  {"openPort", (PyCFunction) MidiOut_openPort, METH_VARARGS,
    "Open a MIDI input connection."},

  {"openVirtualPort", (PyCFunction) MidiOut_openVirtualPort, METH_VARARGS,
    "Create a virtual input port, with optional name, to allow software "
    "connections (OS X and ALSA only)."},

  {"closePort", (PyCFunction) MidiOut_closePort, METH_NOARGS,
    "Close an open MIDI connection (if one exists)."},

  {"getPortCount", (PyCFunction) MidiOut_getPortCount, METH_NOARGS,
    "Return the number of available MIDI output ports."},

  {"getPortName", (PyCFunction) MidiOut_getPortName, METH_VARARGS,
    "Return a string identifier for the specified MIDI port type and number."},

  {"sendMessage", (PyCFunction) MidiOut_sendMessage, METH_VARARGS,
    "Immediately send a single message out an open MIDI output port."},

  {NULL}
};


static PyTypeObject MidiOut_type = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "midi.RtMidiOut",             /*tp_name*/
  sizeof(MidiOut), /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor) MidiOut_dealloc,                         /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  0,                         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
  "Midi output device",           /* tp_doc */
  0,               /* tp_traverse */
  0,               /* tp_clear */
  0,               /* tp_richcompare */
  0,               /* tp_weaklistoffset */
  0,               /* tp_iter */
  0,               /* tp_iternext */
  MidiOut_methods,             /* tp_methods */
  0,              /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)MidiOut_init,      /* tp_init */
  0,                         /* tp_alloc */
  MidiOut_new,                 /* tp_new */
};




static PyMethodDef midi_methods[] = {
  {NULL}  /* Sentinel */
};


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initrtmidi(void)
{
  PyEval_InitThreads();

  PyObject* module;

  if (PyType_Ready(getMidiMessageType()) < 0)
    return;
  if (PyType_Ready(&MidiIn_type) < 0)
    return;
  if (PyType_Ready(&MidiOut_type) < 0)
    return;

  module = Py_InitModule3("rtmidi", midi_methods, "RtMidi wrapper.");

  Py_INCREF((PyObject*) getMidiMessageType());
  PyModule_AddObject(module, "MidiMessage", (PyObject*) getMidiMessageType());

  Py_INCREF(&MidiIn_type);
  PyModule_AddObject(module, "RtMidiIn", (PyObject *)&MidiIn_type);

  Py_INCREF(&MidiOut_type);
  PyModule_AddObject(module, "RtMidiOut", (PyObject *)&MidiOut_type);

  char eName[32];
  strcpy(eName, "rtmidi.Error");
  PyRtMidiError = PyErr_NewException(eName, NULL, NULL);
  Py_INCREF(PyRtMidiError);
  PyModule_AddObject(module, "Error", PyRtMidiError);
}
