/*
 #   Copyright (C) 2014 by Patrick Stinson                                 
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

#include "RtMidi.h"
#include "PyMidiMessage.h"
#include "pkglobals.h"
#include <queue>
  
static PyObject *rtmidi_Error;

typedef struct
{
  PyObject_HEAD;
  
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
#endif
    
} MidiIn;

const char *MidiMessage_str(const MidiMessage *m) {
  static char s[256];
  if(m->isNoteOn()) {
    sprintf(s, "<NOTE ON, %s (note %d), velocity: %d, channel: %d>", m->getMidiNoteName(m->getNoteNumber(), true, true, 0), m->getNoteNumber(), m->getVelocity(), m->getChannel());
  } else if(m->isNoteOff()) {
    sprintf(s, "<NOTE OFF, %s (%d), channel: %d>", m->getMidiNoteName(m->getNoteNumber(), true, true, 0), m->getNoteNumber(), m->getChannel());
  } else if(m->isProgramChange()) {
    sprintf(s, "<PROGRAM CHANGE: program: %d, channel: %d>", m->getProgramChangeNumber(), m->getChannel());
  } else if(m->isPitchWheel()) {
    sprintf(s, "<PITCH WHEEL: value: %d, channel: %d>", m->getPitchWheelValue(), m->getChannel());
  } else if(m->isAftertouch()) {
    sprintf(s, "<AFTERTOUCH: value: %d, channel: %d>", m->getAfterTouchValue(), m->getChannel());
  } else if(m->isChannelPressure()) {
    sprintf(s, "<CHANNEL PRESSURE: pressure: %d, channel: %d>", m->getChannelPressureValue(), m->getChannel());
  } else if(m->isController()) {
    sprintf(s, "<CONTROLLER: \"%s\" (CC %d), value: %d, channel: %d>", m->getControllerName(m->getControllerNumber()), m->getControllerNumber(), m->getControllerValue(), m->getChannel());
  } else {
    sprintf(s, "<MidiMessage (misc type)>");
  }
   return s;
}
  
static void MidiIn_callback(double timestamp, std::vector<unsigned char> *message, void *opaque)
{
  MidiIn *self = (MidiIn *) opaque;
  
  uint8 *data = (uint8*) malloc(message->size());
  for(size_t i=0; i < message->size(); i++)
    data[i] = (*message)[i];  
  MidiMessage inMidi(data, (int) message->size(), timestamp);
  free(data);
  
#if PK_WINDOWS
  WaitForSingleObject(self->mutex, INFINITE);
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
      PyErr_Format(rtmidi_Error, "timeout value must be a number, not %s", timeout->ob_type->tp_name);
      return NULL;
    }
    
    if(ms < 0)
    {
      PyErr_SetString(rtmidi_Error, "timeout value must be a positive number");
      return NULL;
    }
  }
  
#if PK_WINDOWS
  WaitForSingleObject(self->mutex, INFINITE);
#else
  pthread_mutex_lock(&self->mutex);    
#endif

  if(self->m_q->size() == 0 && ms > -1)
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
  RtMidiIn::Api api = RtMidiIn::UNSPECIFIED;
  char *clientName = (char *) ""; // avoid clang warnings;
  unsigned int queueSizeLimit = 100;
  MidiIn *self = NULL;

  if(!PyArg_ParseTuple(args, "|isI", &api, &clientName, &queueSizeLimit))
    return NULL;

  self = (MidiIn *) type->tp_alloc(type, 0);
  
  try
    {
      self->rtmidi = new RtMidiIn(api, clientName, queueSizeLimit);
    }
  catch(RtMidiError &error)
    {
      PyErr_SetString(rtmidi_Error, error.what());
      Py_DECREF(self);
      return NULL;
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
  
#if PK_WINDOWS
  ReleaseMutex(self->mutex);
  CloseHandle (self->mutex);
  CloseHandle (self->cond);
#else
  pthread_mutex_unlock(&self->mutex);  
  pthread_mutex_destroy(&self->mutex);
  pthread_cond_destroy(&self->cond);
#endif

#if PK_PYTHON3
  Py_TYPE(self)->tp_free((PyObject *) self);
#else
  self->ob_type->tp_free((PyObject *) self);
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
      PyErr_SetString(rtmidi_Error, error.what());
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
      PyErr_SetString(rtmidi_Error, error.what());
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
      PyErr_SetString(rtmidi_Error, error.what());
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
      PyErr_SetString(rtmidi_Error, error.what());
      return NULL;
    }
  }
  
  self->rtmidi->setCallback(MidiIn_callback, self);
  self->calling_thread_id = PyThreadState_Get()->thread_id;
  
  Py_RETURN_NONE;
}
  

static PyObject *
MidiIn_isPortOpen(MidiIn *self, PyObject *args)
{
  if(self->rtmidi->isPortOpen())
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
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
  return PK_INT(self->rtmidi->getPortCount());
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
    PyErr_SetString(rtmidi_Error, error.what());
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
  
  midiSysex = PyObject_IsTrue(omidiSysex) == 1;
  midiTime = PyObject_IsTrue(omidiTime) == 1;
  midiSense = PyObject_IsTrue(omidiSense) == 1;
  
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
  
  {"isPortOpen", (PyCFunction) MidiIn_isPortOpen, METH_NOARGS,
    "Returns true if a port is open and false if not."},
  
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
#if ! PK_PYTHON3
  0,                         /*ob_size*/
#endif
  "midi.RtMidiIn",           /*tp_name*/
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
#if PK_PYTHON3
  Py_TYPE(self)->tp_free((PyObject *) self);
#else
  self->ob_type->tp_free((PyObject *) self);
#endif
}


static PyObject *
MidiOut_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  char *name = NULL;

  if(!PyArg_ParseTuple(args, "|s", &name))
    return NULL;

  MidiOut *self = (MidiOut *) type->tp_alloc(type, 0);
  try
    {
      self->rtmidi = new RtMidiOut;
    }
  catch(RtMidiError &error)
    {
      PyErr_SetString(rtmidi_Error, error.what());
      Py_DECREF(self);
      return NULL;
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
      PyErr_SetString(rtmidi_Error, error.what());
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
      PyErr_SetString(rtmidi_Error, error.what());
      return NULL;
    }
  }  
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
MidiOut_isPortOpen(MidiOut *self, PyObject *args)
{
  if(self->rtmidi->isPortOpen())
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
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
      PyErr_SetString(rtmidi_Error, error.what());
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
      PyErr_SetString(rtmidi_Error, error.what());
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
    PyErr_SetString(rtmidi_Error, error.what());
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

  MidiMessage *created = NULL; // delete if set
  MidiMessage *midi = NULL;
  if(PyMidiMessage_Check(a0))
    {
      midi = ((PyMidiMessage *) a0)->m;
    }
  else if(PyLong_Check(a0))
    {
      PyErr_SetString(rtmidi_Error, "long ctor args not supported yet.");
      return NULL;
      /*
      int i0 = (int) PyLong_AsUnsignedLong(a0);
      printf("MidiOut_sendMessage: %i\n", i0);
      midi = created = new MidiMessage(i0);
      */
    }
  else
    {
      PyErr_SetString(rtmidi_Error, "argument 1 must be of type MidiMessage or a number.");
      return NULL;
    }
  
  std::vector<unsigned char> outMessage;
  uint8 *data = midi->getRawData();
  for(int i=0; i < midi->getRawDataSize(); ++i)
    outMessage.push_back((unsigned char) data[i]);
  
  try
  {
    self->rtmidi->sendMessage(&outMessage);
  }
  catch(RtMidiError &error)
  {
    PyErr_SetString(rtmidi_Error, error.what());
    if(created) delete created;
    return NULL;
  }
  
  if(created) delete created;
  Py_RETURN_NONE;
}

static PyMethodDef MidiOut_methods[] = {
  {"openPort", (PyCFunction) MidiOut_openPort, METH_VARARGS,
    "Open a MIDI input connection."},
  
  {"openVirtualPort", (PyCFunction) MidiOut_openVirtualPort, METH_VARARGS,
    "Create a virtual input port, with optional name, to allow software "
    "connections (OS X and ALSA only)."},
  
  {"isPortOpen", (PyCFunction) MidiOut_isPortOpen, METH_NOARGS,
    "Returns true if a port is open and false if not."},
  
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
#if ! PK_PYTHON3
  0,                         /*ob_size*/
#endif
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


#if PK_PYTHON3
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "rtmidi",     /* m_name */
    "RtMidi wrapper",  /* m_doc */
    -1,                  /* m_size */
    midi_methods,    /* m_methods */
    NULL,                /* m_reload */
    NULL,                /* m_traverse */
    NULL,                /* m_clear */
    NULL,                /* m_free */
};
#endif


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif


#if PK_PYTHON3
PyMODINIT_FUNC PyInit__rtmidi(void)
#else
PyMODINIT_FUNC init_rtmidi(void) 
#endif
{
  PyEval_InitThreads();
  
  PyObject* module;
  
  if (PyType_Ready(getMidiMessageType()) < 0)
#if PK_PYTHON3
    return 0;
#else
    return;
#endif
  if (PyType_Ready(&MidiIn_type) < 0)
#if PK_PYTHON3
    return 0;
#else
    return;
#endif
  if (PyType_Ready(&MidiOut_type) < 0)
#if PK_PYTHON3
    return 0;
#else
    return;
#endif
  
#if PK_PYTHON3
  module = PyModule_Create(&moduledef);
#else
  module = Py_InitModule3("rtmidi._rtmidi", midi_methods, "RtMidi wrapper");
#endif
  
  Py_INCREF((PyObject*) getMidiMessageType());
  PyModule_AddObject(module, "MidiMessage", (PyObject*) getMidiMessageType());
    
  Py_INCREF(&MidiIn_type);
  PyModule_AddObject(module, "RtMidiIn", (PyObject *)&MidiIn_type);
  
  Py_INCREF(&MidiOut_type);
  PyModule_AddObject(module, "RtMidiOut", (PyObject *)&MidiOut_type);

  PyObject *inDict = MidiIn_type.tp_dict;
  PyObject *outDict = MidiOut_type.tp_dict;

  PyDict_SetItemString(inDict, "UNSPECIFIED", PyLong_FromLong(RtMidi::UNSPECIFIED));
  PyDict_SetItemString(outDict, "UNSPECIFIED", PyLong_FromLong(RtMidi::UNSPECIFIED));
  PyDict_SetItemString(inDict, "MACOSX_CORE", PyLong_FromLong(RtMidi::MACOSX_CORE));
  PyDict_SetItemString(outDict, "MACOSX_CORE", PyLong_FromLong(RtMidi::MACOSX_CORE));
  PyDict_SetItemString(inDict, "LINUX_ALSA", PyLong_FromLong(RtMidi::LINUX_ALSA));
  PyDict_SetItemString(outDict, "LINUX_ALSA", PyLong_FromLong(RtMidi::LINUX_ALSA));
  PyDict_SetItemString(inDict, "UNIX_JACK", PyLong_FromLong(RtMidi::UNIX_JACK));
  PyDict_SetItemString(outDict, "UNIX_JACK", PyLong_FromLong(RtMidi::UNIX_JACK));
  PyDict_SetItemString(inDict, "WINDOWS_MM", PyLong_FromLong(RtMidi::WINDOWS_MM));
  PyDict_SetItemString(outDict, "WINDOWS_MM", PyLong_FromLong(RtMidi::WINDOWS_MM));
  PyDict_SetItemString(inDict, "RTMIDI_DUMMY", PyLong_FromLong(RtMidi::RTMIDI_DUMMY));
  PyDict_SetItemString(outDict, "RTMIDI_DUMMY", PyLong_FromLong(RtMidi::RTMIDI_DUMMY));
  
  char eName[32];
#if PK_WINDOWS
  strcpy_s(eName, "rtmidi.Error");
#else
  strcpy(eName, "rtmidi.Error");
#endif
  rtmidi_Error = PyErr_NewException(eName, NULL, NULL);
  Py_INCREF(rtmidi_Error);
  PyModule_AddObject(module, "Error", rtmidi_Error);  

#if PK_PYTHON3
    return module;
#endif  
}

