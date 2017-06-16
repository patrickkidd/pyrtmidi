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


#include <Python.h>
#include "pkglobals.h"
#include "PyMidiMessage.h"


#define __midi ((PyMidiMessage *)self)->m


PyObject *PyMidiMessage_FromMidiMessage(const MidiMessage &m)
{
  PyMidiMessage *result = (PyMidiMessage *) PyMidiMessage_new();
  *(result->m) = m;
  return (PyObject *) result;
}


// MidiMessage class wrapper

static void
PyMidiMessage_dealloc(PyMidiMessage* self)
{
  delete self->m;
#if PK_PYTHON3
  Py_TYPE(self)->tp_free((PyObject*)self);
#else
  self->ob_type->tp_free((PyObject*)self);
#endif
}

static PyObject *
PyMidiMessage_new(PyTypeObject *type, PyObject *args, PyObject *)
{
  PyMidiMessage *self;
  PyObject *bytes = NULL;
  char *bytesStr = NULL;
  int iNumBytes = NULL;
  double timeStamp = NULL;

  PyObject *a1 = NULL, *a2 = NULL, *a3 = NULL;

  self = (PyMidiMessage *)type->tp_alloc(type, 0);
  if(self == NULL)
    return NULL;

  if(args) {
    if(PyTuple_GET_SIZE(args) == 1) { // either bytes as raw data or copy ctor
      a1 = PyTuple_GET_ITEM(args, 0);
      if(PyBytes_Check(a1)) {
        bytes = a1;
        bytesStr = PyBytes_AsString(bytes);
        iNumBytes = PyBytes_GET_SIZE(bytes);
        self->m = new MidiMessage((const uint8 *) bytesStr, (const int) iNumBytes);
      } else if(PyMidiMessage_Check(a1)) {
        self->m = new MidiMessage(0xb0, 123 & 127, 0); // all notes off (dummy)
        PyMidiMessage *pyOther = (PyMidiMessage *) a1;
        *(self->m) = *(pyOther->m);
      } else {
        PyArg_ParseTuple(args, "O|O", &a1, &a2, &a3); // just set exception
        return NULL;
      }
    } else if(PyTuple_GET_SIZE(args) == 2) {
      if(!PyArg_ParseTuple(args, "Sd", &bytes, &timeStamp)) {
        return NULL;
      }
      bytesStr = PyBytes_AsString(bytes);
      iNumBytes = PyBytes_GET_SIZE(bytes);
      self->m = new MidiMessage((const uint8 *) bytesStr, (const int) iNumBytes, (const double) timeStamp);
    } else {
      self->m = new MidiMessage(0xb0, 123 & 127, 0); // all notes off
    }
  } else {
      self->m = new MidiMessage(0xb0, 123 & 127, 0); // all notes off
  }
  
  return (PyObject *)self;

/*      
      a1 = PyTuple_GET_ITEM(args, 0);
      if(PyBytes_Check(a1)) {
        char *s = PyBytes_AsString(a1);
      }
      PyArg_ParseTuple(args, "S", data, &timeStamp);
    } else if(PyTuple_GET_SIZE(args) == 2) {
      if(PyBytes_Check(a1)) {
        int size = PyBytes_GET_SIZE(a1);
        char *s = PyBytes_AsString(a1);
        self->m = new MidiMessage((const uint8 *) s, (const int) size, (const double) timeStamp);
      }
      
        self->m = new MidiMessage((const uint8 *) s, (const int) size, (const double) timeStamp);
      PyArg_ParseTuple(args, "Sd", data, &timeStamp);
    }
  }

  if(args) {
    printf("here: %p, %i %i\n", args, PyArg_ParseTuple(args, "Si|d", &data, &iNumBytes, &timeStamp), PyArg_ParseTuple(args, "|O", &other));
    if(!PyArg_ParseTuple(args, "Si|d", &data, &iNumBytes, &timeStamp) &&
       !PyArg_ParseTuple(args, "|O", &other)) {
      return NULL;
    }
  } else {
    printf("here: %p\n", args);
  }  
  
  // create from default ctor
  if(args == NULL) {
    self->m = new MidiMessage(0xb0, 123 & 127, 0); // all notes off
  } else {
    // create from raw data
    if(PyArg_ParseTuple(args, "Si|d", &data, &iNumBytes, &timeStamp)) {
      char *s = PyByteArray_AsString(data);
      if(timeStamp) {
        self->m = new MidiMessage((const uint8 *) s, (const int) iNumBytes, (const double) timeStamp);
      } else {
        self->m = new MidiMessage((const uint8 *) s, (const int) iNumBytes);
      }
    } else if(PyArg_ParseTuple(args, "|O", &other)) {
      // create from copy ctor
      if(other) {
        if(PyMidiMessage_Check(other)) {
      printf("here 1\n");
          self->m = new MidiMessage(0xb0, 123 & 127, 0); // all notes off (dummy)
          PyMidiMessage *pyOther = (PyMidiMessage *) other;
          *self->m = *pyOther->m;
      printf("here 2: %p\n", self);
        } else {
          PyErr_SetString(PyExc_ValueError, "copy constructor argument must be a MidiMessage.");
          Py_DECREF(self);
          self = NULL;
        }
      }
    }
  }

  printf("return: %p\n", self);
  return (PyObject *)self;
*/
}

static int
PyMidiMessage_init(MidiMessage *, PyObject *, PyObject *)
{
  return 0;
}




static PyObject *
PyMidiMessage_getTimeStamp(PyObject *self, PyObject *)
{
  return PK_FLOAT(__midi->getTimeStamp());
}

static PyObject *
PyMidiMessage_setTimeStamp(PyObject *self, PyObject *args)
{
  double ts;
  if(!PyArg_ParseTuple(args, "d", &ts))
    return NULL;
  
  __midi->setTimeStamp(ts);
  
  Py_RETURN_NONE;
}


static PyObject *
PyMidiMessage_addToTimeStamp(PyObject *self, PyObject *args)
{
  double ts;
  if(!PyArg_ParseTuple(args, "d", &ts))
    return NULL;
  
  __midi->addToTimeStamp(ts);
  
  Py_RETURN_NONE;
}


static PyObject *
PyMidiMessage_getChannel(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getChannel());
}


static PyObject *
PyMidiMessage_isForChannel(PyObject *self, PyObject *args)
{
  int x;
  if(!PyArg_ParseTuple(args, "i", &x))
    return NULL;
  
  return PK_BOOL(__midi->isForChannel(x));
}

static PyObject *
PyMidiMessage_setChannel(PyObject *self, PyObject *args)
{
  int x;
  if(!PyArg_ParseTuple(args, "i", &x))
    return NULL;
  
  __midi->setChannel(x);
  
  Py_RETURN_NONE;
}

static PyObject *
PyMidiMessage_isSysEx(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isSysEx());
}

static PyObject *
PyMidiMessage_isNoteOn(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isNoteOn());
}

static PyObject *
PyMidiMessage_isNoteOff(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isNoteOff());
}

static PyObject *
PyMidiMessage_isNoteOnOrOff(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isNoteOnOrOff());
}

static PyObject *
PyMidiMessage_getNoteNumber(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getNoteNumber());
}

static PyObject *
PyMidiMessage_setNoteNumber(PyObject *self, PyObject *args)
{
  int note;
  if(!PyArg_ParseTuple(args, "i", &note))
    return NULL;
  
  __midi->setNoteNumber(note);
  
  Py_RETURN_NONE;
}

static PyObject *
PyMidiMessage_getVelocity(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getVelocity());
}

static PyObject *
PyMidiMessage_getFloatVelocity(PyObject *self, PyObject *)
{
  return PK_FLOAT(__midi->getFloatVelocity());
}


static PyObject *
PyMidiMessage_setVelocity(PyObject *self, PyObject *args)
{
  float x;
  if(!PyArg_ParseTuple(args, "f", &x))
    return NULL;
  
  __midi->setVelocity(x);
  
  Py_RETURN_NONE;
}

static PyObject *
PyMidiMessage_multiplyVelocity(PyObject *self, PyObject *args)
{
  float x;
  if(!PyArg_ParseTuple(args, "f", &x))
    return NULL;
  
  __midi->multiplyVelocity(x);
  
  Py_RETURN_NONE;
}

static PyObject *
PyMidiMessage_isProgramChange(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isProgramChange());
}

static PyObject *
PyMidiMessage_getProgramChangeNumber(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getProgramChangeNumber());
}

static PyObject *
PyMidiMessage_isPitchWheel(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isPitchWheel());
}


static PyObject *
PyMidiMessage_getPitchWheelValue(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getPitchWheelValue());
}

static PyObject *
PyMidiMessage_isAftertouch(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isAftertouch());
}

static PyObject *
PyMidiMessage_getAfterTouchValue(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getAfterTouchValue());
}

static PyObject *
PyMidiMessage_isChannelPressure(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isChannelPressure());
}

static PyObject *
PyMidiMessage_getChannelPressureValue(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getChannelPressureValue());
}

static PyObject *
PyMidiMessage_isController(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isController());
}

static PyObject *
PyMidiMessage_getControllerNumber(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getControllerNumber());
}


static PyObject *
PyMidiMessage_getControllerValue(PyObject *self, PyObject *)
{
  return PK_INT(__midi->getControllerValue());
}

static PyObject *
PyMidiMessage_isAllNotesOff(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isAllNotesOff());
}

static PyObject *
PyMidiMessage_isAllSoundOff(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isAllSoundOff());
}

static PyObject *
PyMidiMessage_isActiveSense(PyObject *self, PyObject *)
{
  return PK_BOOL(__midi->isAllSoundOff());
}


static PyObject *
PyMidiMessage_getRawData(PyMidiMessage *self, PyObject *)
{
#if PK_PYTHON3
  return PyBytes_FromStringAndSize((const char *) __midi->getRawData(), __midi->getRawDataSize());
#else
  return PyString_FromStringAndSize((const char *) __midi->getRawData(), __midi->getRawDataSize());
#endif  
}

static PyObject *
PyMidiMessage_getRawDataSize(PyMidiMessage *self, PyObject *)
{
  return PK_INT(__midi->getRawDataSize());
}

static PyObject *
PyMidiMessage_getSysExData(PyMidiMessage *self, PyObject *)
{
#if PK_PYTHON3
  return PyBytes_FromStringAndSize((const char *) __midi->getSysExData(), __midi->getSysExDataSize());
#else
  return PyString_FromStringAndSize((const char *) __midi->getSysExData(), __midi->getSysExDataSize());
#endif  
}

static PyObject *
PyMidiMessage_noteOn(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int noteNumber = -1;
  uint8 velocity = -1;
  if(!PyArg_ParseTuple(args, "iiH", &channel, &noteNumber, &velocity))
    return NULL;
    
  PyObject *pyNoteNumber = PyTuple_GetItem(args, 1);
  noteNumber = (int) PyLong_AsLong(pyNoteNumber); // no idea why PyArg_ParseTuple is setting *only* noteNumber to zero...
  return PyMidiMessage_FromMidiMessage(MidiMessage::noteOn(channel, noteNumber, velocity));
}

static PyObject *
PyMidiMessage_noteOff(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int noteNumber = -1;
  if(!PyArg_ParseTuple(args, "ii", &channel, &noteNumber))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::noteOff(channel, noteNumber));
}

static PyObject *
PyMidiMessage_programChange(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int programNumber = -1;
  if(!PyArg_ParseTuple(args, "ii", &channel, &programNumber))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::programChange(channel, programNumber));
}


static PyObject *
PyMidiMessage_pitchWheel(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int position = -1;
  if(!PyArg_ParseTuple(args, "ii", &channel, &position))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::pitchWheel(channel, position));
}

static PyObject *
PyMidiMessage_aftertouchChange(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int noteNumber = -1;
  int aftertouchAmount = -1;
  if(!PyArg_ParseTuple(args, "iii", &channel, &noteNumber, &aftertouchAmount))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::aftertouchChange(channel, noteNumber, aftertouchAmount));
}

static PyObject *
PyMidiMessage_channelPressureChange(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int pressure = -1;
  if(!PyArg_ParseTuple(args, "ii", &channel, &pressure))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::channelPressureChange(channel, pressure));
}

static PyObject *
PyMidiMessage_controllerEvent(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  int controllerType = -1;
  int value = -1;
  if(!PyArg_ParseTuple(args, "iii", &channel, &controllerType, &value))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::controllerEvent(channel, controllerType, value));
}

static PyObject *
PyMidiMessage_allNotesOff(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  if(!PyArg_ParseTuple(args, "i", &channel))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::allNotesOff(channel));
}

static PyObject *
PyMidiMessage_allSoundOff(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  if(!PyArg_ParseTuple(args, "i", &channel))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::allSoundOff(channel));
}

static PyObject *
PyMidiMessage_allControllersOff(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  if(!PyArg_ParseTuple(args, "i", &channel))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::allControllersOff(channel));
}

static PyObject *
PyMidiMessage_endOfTrack(PyMidiMessage *, PyObject *)
{
  return PyMidiMessage_FromMidiMessage(MidiMessage::endOfTrack());
}

static PyObject *
PyMidiMessage_tempoMetaEvent(PyMidiMessage *, PyObject *args)
{
  int microsecondsPerQuarterNote = -1;
  if(!PyArg_ParseTuple(args, "i", &microsecondsPerQuarterNote))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::tempoMetaEvent(microsecondsPerQuarterNote));
}

static PyObject *
PyMidiMessage_timeSignatureMetaEvent(PyMidiMessage *, PyObject *args)
{
  int numerator = -1;
  int denominator = -1;
  if(!PyArg_ParseTuple(args, "ii", &numerator, &denominator))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::timeSignatureMetaEvent(numerator, denominator));
}

static PyObject *
PyMidiMessage_midiChannelMetaEvent(PyMidiMessage *, PyObject *args)
{
  int channel = -1;
  if(!PyArg_ParseTuple(args, "i", &channel))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiChannelMetaEvent(channel));
}

static PyObject *
PyMidiMessage_midiStart(PyMidiMessage *, PyObject *)
{
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiStart());
}

static PyObject *
PyMidiMessage_midiContinue(PyMidiMessage *, PyObject *)
{
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiContinue());
}

static PyObject *
PyMidiMessage_midiStop(PyMidiMessage *, PyObject *)
{
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiStop());
}

static PyObject *
PyMidiMessage_midiClock(PyMidiMessage *, PyObject *)
{
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiClock());
}

static PyObject *
PyMidiMessage_songPositionPointer(PyMidiMessage *, PyObject *args)
{
  int positionInMidiBeats = -1;
  if(!PyArg_ParseTuple(args, "i", &positionInMidiBeats))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::songPositionPointer(positionInMidiBeats));
}

static PyObject *
PyMidiMessage_quarterFrame(PyMidiMessage *, PyObject *args)
{
  int sequenceNumber = -1;
  int value = -1;
  if(!PyArg_ParseTuple(args, "ii", &sequenceNumber, &value))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::quarterFrame(sequenceNumber, value));
}

static PyObject *
PyMidiMessage_fullFrame(PyMidiMessage *, PyObject *args)
{
  int sequenceNumber = -1;
  int value = -1;
  
  if(!PyArg_ParseTuple(args, "iiiii", &sequenceNumber, &value))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::quarterFrame(sequenceNumber, value));
}

static PyObject *
PyMidiMessage_midiMachineControlCommand(PyMidiMessage *, PyObject *args)
{
  MidiMessage::MidiMachineControlCommand command;
  
  if(!PyArg_ParseTuple(args, "i", &command))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiMachineControlCommand(command));
}

static PyObject *
PyMidiMessage_midiMachineControlGoto(PyMidiMessage *, PyObject *args)
{
  int hours = -1;
  int minutes = -1;
  int seconds = -1;
  int frames = -1;
  
  if(!PyArg_ParseTuple(args, "iiii", &hours, &minutes, &seconds, &frames))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::midiMachineControlGoto(hours, minutes, seconds, frames));
}

static PyObject *
PyMidiMessage_masterVolume(PyMidiMessage *, PyObject *args)
{
  float volume;
  
  if(!PyArg_ParseTuple(args, "f", &volume))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::masterVolume(volume));
}


static PyObject *
PyMidiMessage_createSysExMessage(PyMidiMessage *, PyObject *args)
{
  const char *data = NULL;
  int dataSize = -1;
  
  if(!PyArg_ParseTuple(args, "y#", &data, &dataSize))
    return NULL;
  
  return PyMidiMessage_FromMidiMessage(MidiMessage::createSysExMessage((const uint8 *) data, dataSize));
}

static PyObject *
PyMidiMessage_readVariableLengthVal(PyMidiMessage *, PyObject *args)
{
  const char *data = NULL;
  int dataSize = -1;
  
  if(!PyArg_ParseTuple(args, "y#", &data, &dataSize))
    return NULL;
  
  return PyLong_FromLong(MidiMessage::readVariableLengthVal((const uint8 *)data, dataSize));
}

static PyObject *
PyMidiMessage_getMessageLengthFromFirstByte(PyMidiMessage *, PyObject *args)
{
  const char *data = NULL;
  int dataSize = -1;
  
  if(!PyArg_ParseTuple(args, "y#", &data, &dataSize))
    return NULL;
  
  if(dataSize == 0)
    return PyLong_FromLong(0);
  else
    return PyLong_FromLong(MidiMessage::getMessageLengthFromFirstByte((const uint8)data[0]));
}


static PyObject *
PyMidiMessage_getMidiNoteName(PyObject *, PyObject *args)
{
  int noteNumber;
  bool useSharps = true;
  bool includeOctaveNumber = true;
  int octaveNumForMiddleC = 3;
  
  if(!PyArg_ParseTuple(args, "i|bbi", &noteNumber, &useSharps, &includeOctaveNumber, &octaveNumForMiddleC))
    return NULL;
  
	return PK_STRING(MidiMessage::getMidiNoteName(noteNumber, useSharps, includeOctaveNumber, octaveNumForMiddleC));
}

static PyObject *
PyMidiMessage_getMidiNoteInHertz(PyObject *, PyObject *args)
{
  int noteNumber;
  
  if(!PyArg_ParseTuple(args, "i", &noteNumber))
    return NULL;
  
  return PK_FLOAT(MidiMessage::getMidiNoteInHertz(noteNumber));
}

static PyObject *
PyMidiMessage_getGMInstrumentName(PyObject *, PyObject *args)
{
  int midiInstrumentNumber;
  
  if(!PyArg_ParseTuple(args, "i", &midiInstrumentNumber))
    return NULL;
  
	return PK_STRING(MidiMessage::getGMInstrumentName(midiInstrumentNumber));
}

static PyObject *
PyMidiMessage_getGMInstrumentBankName(PyObject *, PyObject *args)
{
  int midiBankNumber;
  
  if(!PyArg_ParseTuple(args, "i", &midiBankNumber))
    return NULL;
  
	return PK_STRING(MidiMessage::getGMInstrumentBankName(midiBankNumber));
}

static PyObject *
PyMidiMessage_getRhythmInstrumentName(PyObject *, PyObject *args)
{
  int midiNoteNumber;
  
  if(!PyArg_ParseTuple(args, "i", &midiNoteNumber))
    return NULL;
  
	return PK_STRING(MidiMessage::getRhythmInstrumentName(midiNoteNumber));
}


static PyObject *
PyMidiMessage_getControllerName(PyObject *, PyObject *args)
{
  int controllerNumber;
  
  if(!PyArg_ParseTuple(args, "i", &controllerNumber))
    return NULL;
  
	return PK_STRING(MidiMessage::getControllerName(controllerNumber));
}

static PyObject *
PyMidiMessage_str(PyObject *self) {
  MidiMessage *m = ((PyMidiMessage*)self)->m;
  static char s[256];
  if(m->isNoteOn()) {
    sprintf(s, "<NOTE ON, note: %d (%s), velocity: %d, channel: %d>",
            m->getNoteNumber(),
            m->getMidiNoteName(m->getNoteNumber(), true, true, 3),
            m->getVelocity(),
            m->getChannel());
  } else if(m->isNoteOff()) {
    sprintf(s, "<NOTE OFF, note: %d (%s), channel: %d>",
            m->getNoteNumber(),
            m->getMidiNoteName(m->getNoteNumber(), true, true, 3),
            m->getChannel());
  } else if(m->isProgramChange()) {
    sprintf(s, "<PROGRAM CHANGE: program: %d, channel: %d>", m->getProgramChangeNumber(), m->getChannel());
  } else if(m->isPitchWheel()) {
    sprintf(s, "<PITCH WHEEL: value: %d, channel: %d>", m->getPitchWheelValue(), m->getChannel());
  } else if(m->isAftertouch()) {
    sprintf(s, "<AFTERTOUCH: note: %d (%s) value: %d, channel: %d>",
            m->getNoteNumber(),
            m->getMidiNoteName(m->getNoteNumber(), true, true, 3),
            m->getAfterTouchValue(),
            m->getChannel());
  } else if(m->isChannelPressure()) {
    sprintf(s, "<CHANNEL PRESSURE: pressure: %d, channel: %d>", m->getChannelPressureValue(), m->getChannel());
  } else if(m->isController()) {
    const char *name = m->getControllerName(m->getControllerNumber());
    if(strlen(name) > 0) {
      sprintf(s, "<CONTROLLER: %d (\"%s\"), value: %d, channel: %d>",
              m->getControllerNumber(),
              m->getControllerName(m->getControllerNumber()),
              m->getControllerValue(),
              m->getChannel());
    } else {
      sprintf(s, "<CONTROLLER: %d, value: %d, channel: %d>",
              m->getControllerNumber(),
              m->getControllerValue(),
              m->getChannel());
    }
  } else {
    sprintf(s, "<MidiMessage (misc type)>");
  }

  return PK_STRING(s);
}

PyObject *PyMidiMessage___eq__(PyObject *self, PyObject *other, int op) {

  PyObject *result = NULL;

  if(other == Py_None) {
    result = Py_False;
  } else if(!PyMidiMessage_Check(other)) {
    result = Py_False;
  } else {
    MidiMessage &me = *((PyMidiMessage *) self)->m;
    MidiMessage &him = *((PyMidiMessage *) other)->m;
    switch (op) {
    case Py_EQ:
      result = (me == him) ? Py_True : Py_False;
      break;
    case Py_NE:
      result = (me == him) ? Py_False : Py_True;
      break;
    case Py_LT:
    case Py_LE:
    case Py_GT:
    case Py_GE:
    default:
      result = Py_NotImplemented;
      break;
    }
 }

  Py_XINCREF(result);
  return result;
}

/* TODO: 
 
  getSysExData()
  getSysExDataSize()
  programChange()
  pitchWheel()
  afterTouchChange()
  channelPressureChange()
  controllerEvent()
  allNotesOff()
  allSoundOff()
  allControllersOff()
  Meta Events
 */


static PyMethodDef PyMidiMessage_methods[] = {
  {"getTimeStamp", (PyCFunction) PyMidiMessage_getTimeStamp, METH_NOARGS,
    "Returns the timestamp associated with this message." },
  {"setTimeStamp", (PyCFunction) PyMidiMessage_setTimeStamp, METH_VARARGS,
    "Changes the message's associated timestamp." },
  {"addToTimeStamp", (PyCFunction) PyMidiMessage_addToTimeStamp, METH_VARARGS,
    "Adds a value to the message's timestamp." },
  {"getChannel", (PyCFunction) PyMidiMessage_getChannel, METH_NOARGS,
    "Returns true if the message applies to the given midi channel." },
  {"isForChannel", (PyCFunction) PyMidiMessage_isForChannel, METH_VARARGS,
    "Returns true if the message applies to the given midi channel." },
  {"setChannel", (PyCFunction) PyMidiMessage_setChannel, METH_VARARGS,
    "Changes the message's midi channel." },
  {"isSysEx", (PyCFunction) PyMidiMessage_isSysEx, METH_NOARGS,
    "Returns true if this is a system-exclusive message." },
  {"isNoteOn", (PyCFunction) PyMidiMessage_isNoteOn, METH_NOARGS,
    "Returns true if this message is a 'key-down' event." },
  {"isNoteOff", (PyCFunction) PyMidiMessage_isNoteOff, METH_NOARGS,
    "Returns true if this message is a 'key-up' event." },
  {"isNoteOnOrOff", (PyCFunction) PyMidiMessage_isNoteOnOrOff, METH_NOARGS,
    "Returns true if this message is a 'key-down' or 'key-up' event." },
  {"getNoteNumber", (PyCFunction) PyMidiMessage_getNoteNumber, METH_NOARGS,
    "Returns the midi note number for note-on and note-off messages." },
  {"setNoteNumber", (PyCFunction) PyMidiMessage_setNoteNumber, METH_VARARGS,
    "Changes the midi note number of a note-on or note-off message." },
  {"getVelocity", (PyCFunction) PyMidiMessage_getVelocity, METH_NOARGS,
    "Returns the velocity of a note-on or note-off message (0 - 127, or 0 for non-note events)." },
  {"getFloatVelocity", (PyCFunction) PyMidiMessage_getFloatVelocity, METH_NOARGS,
    "Returns the velocity of a note-on or note-off message (0.0 - 1.0, or 0 for non-note events)." },
  {"setVelocity", (PyCFunction) PyMidiMessage_setVelocity, METH_VARARGS,
    "Changes the velocity of a note-on or note-off message (0.0 - 1.0)" },
  {"multiplyVelocity", (PyCFunction) PyMidiMessage_multiplyVelocity, METH_VARARGS,
    "Multiplies the velocity of a note-on or note-off message by a given float amount." },
  {"isProgramChange", (PyCFunction) PyMidiMessage_isProgramChange, METH_NOARGS,
    "Returns true if the message is a program (patch) change message." },
  {"getProgramChangeNumber", (PyCFunction) PyMidiMessage_getProgramChangeNumber, METH_NOARGS,
    "Returns the new program number of a program change message." },
  {"isPitchWheel", (PyCFunction) PyMidiMessage_isPitchWheel, METH_NOARGS,
    "Returns true if the message is a pitch-wheel move." },
  {"getPitchWheelValue", (PyCFunction) PyMidiMessage_getPitchWheelValue, METH_NOARGS,
    "Returns the pitch wheel position from a pitch-wheel move message.\n"
    "The value returned is a 14-bit number from 0 to 0x3fff, indicating the wheel position.\n"
    "If called for messages which aren't pitch wheel events, the number returned will be\n"
    "nonsense.\n" },
  {"isAftertouch", (PyCFunction) PyMidiMessage_isAftertouch, METH_NOARGS,
    "Returns true if the message is an aftertouch event.\n"
    "\n"
    "For aftertouch events, use the getNoteNumber() method to find out the key\n"
    "that it applies to, and getAfterTouchValue() to find out the amount. Use\n"
    "getChannel() to find out the channel.\n" },
  {"getAfterTouchValue", (PyCFunction) PyMidiMessage_getAfterTouchValue, METH_NOARGS,
    "Returns the amount of aftertouch from an aftertouch messages.\n"
    "\n"
    "The value returned is in the range 0 to 127, and will be nonsense for messages\n"
    "other than aftertouch messages.\n" },
  {"isChannelPressure", (PyCFunction) PyMidiMessage_isChannelPressure, METH_NOARGS,
    "Returns the pressure from a channel pressure change message." },
  {"getChannelPressureValue", (PyCFunction) PyMidiMessage_getChannelPressureValue, METH_VARARGS,
    "Creates a channel-pressure change event. (0 - 127 )" },
  {"isController", (PyCFunction) PyMidiMessage_isController, METH_NOARGS,
    "Returns true if this is a midi controller message." },
  {"getControllerNumber", (PyCFunction) PyMidiMessage_getControllerNumber, METH_NOARGS,
    "Returns the controller value from a controller message (0 - 127)." },
  {"getControllerValue", (PyCFunction) PyMidiMessage_getControllerValue, METH_NOARGS,
    "Returns the controller value from a controller message (0 - 127)." },
  {"isAllNotesOff", (PyCFunction) PyMidiMessage_isAllNotesOff, METH_NOARGS,
    "Checks whether this message is an all-notes-off message." },
  {"isAllSoundOff", (PyCFunction) PyMidiMessage_isAllSoundOff, METH_NOARGS,
    "Checks whether this message is an all-sound-off message." },
  {"isActiveSense", (PyCFunction) PyMidiMessage_isActiveSense, METH_NOARGS,
    "Returns true if this is an active-sense message." },
  {"getRawData", (PyCFunction) PyMidiMessage_getRawData, METH_NOARGS,
    "Returns the raw midi data." },
  {"getRawDataSize", (PyCFunction) PyMidiMessage_getRawDataSize, METH_NOARGS,
    "Returns the raw midi data size." },
  {"getSysExData", (PyCFunction) PyMidiMessage_getSysExData, METH_NOARGS,
    "Returns the raw midi data." }, 
  
  {"noteOff", (PyCFunction) PyMidiMessage_noteOff, METH_VARARGS | METH_STATIC,
    "Creates a noteOff message." },  
  {"noteOn", (PyCFunction) PyMidiMessage_noteOn, METH_VARARGS | METH_STATIC,
    "Creates a noteOn message." },  
  {"programChange", (PyCFunction) PyMidiMessage_programChange, METH_VARARGS | METH_STATIC,
    "Creates a program-change message." },  
  {"pitchWheel", (PyCFunction) PyMidiMessage_pitchWheel, METH_VARARGS | METH_STATIC,
    "Creates a pitch-wheel move message." },  
  {"aftertouchChange", (PyCFunction) PyMidiMessage_aftertouchChange, METH_VARARGS | METH_STATIC,
    "Creates an aftertouch message." },
  {"channelPressureChange", (PyCFunction) PyMidiMessage_channelPressureChange, METH_VARARGS | METH_STATIC,
    "Creates a channel-pressure change event." },
  {"controllerEvent", (PyCFunction) PyMidiMessage_controllerEvent, METH_VARARGS | METH_STATIC,
    "Creates a controller message." },
  {"allNotesOff", (PyCFunction) PyMidiMessage_allNotesOff, METH_VARARGS | METH_STATIC,
    "Creates an all-notes-off message." },
  {"allSoundOff", (PyCFunction) PyMidiMessage_allSoundOff, METH_VARARGS | METH_STATIC,
    "Creates an all-sound-off message." },
  {"allControllersOff", (PyCFunction) PyMidiMessage_allControllersOff, METH_VARARGS | METH_STATIC,
    "Creates an all-controllers-off message." },
  {"endOfTrack", (PyCFunction) PyMidiMessage_endOfTrack, METH_NOARGS | METH_STATIC,
    "Creates an end-of-track meta-event." },
  {"tempoMetaEvent", (PyCFunction) PyMidiMessage_tempoMetaEvent, METH_VARARGS | METH_STATIC,
    "Creates a tempo meta-event." },
  {"timeSignatureMetaEvent", (PyCFunction) PyMidiMessage_timeSignatureMetaEvent, METH_VARARGS | METH_STATIC,
    "Creates a time-signature meta-event." },
  {"midiChannelMetaEvent", (PyCFunction) PyMidiMessage_midiChannelMetaEvent, METH_VARARGS | METH_STATIC,
    "Creates a midi channel meta-event." },
  {"midiStart", (PyCFunction) PyMidiMessage_midiStart, METH_NOARGS | METH_STATIC,
    "Creates a midi start event." },
  {"midiContinue", (PyCFunction) PyMidiMessage_midiContinue, METH_NOARGS | METH_STATIC,
    "Creates a midi continue event." },
  {"midiStop", (PyCFunction) PyMidiMessage_midiStop, METH_NOARGS | METH_STATIC,
    "Creates a midi stop event." },
  {"midiClock", (PyCFunction) PyMidiMessage_midiClock, METH_NOARGS | METH_STATIC,
    "Creates a midi clock event." },
  {"songPositionPointer", (PyCFunction) PyMidiMessage_songPositionPointer, METH_VARARGS | METH_STATIC,
    "Creates a song-position-pointer message." },  
  {"quarterFrame", (PyCFunction) PyMidiMessage_quarterFrame, METH_VARARGS | METH_STATIC,
    "Creates a quarter-frame MTC message." }, 
  {"fullFrame", (PyCFunction) PyMidiMessage_fullFrame, METH_VARARGS | METH_STATIC,
    "Creates a full-frame MTC message." },
  {"midiMachineControlCommand", (PyCFunction) PyMidiMessage_midiMachineControlCommand, METH_VARARGS | METH_STATIC,
    "Creates an MMC message." },
  {"midiMachineControlGoto", (PyCFunction) PyMidiMessage_midiMachineControlGoto, METH_VARARGS | METH_STATIC,
    "Creates an MMC \"goto\" message." },
  {"masterVolume", (PyCFunction) PyMidiMessage_masterVolume, METH_VARARGS | METH_STATIC,
    "Creates a master-volume change message." },
  {"createSysExMessage", (PyCFunction) PyMidiMessage_createSysExMessage, METH_VARARGS | METH_STATIC,
    "Creates a system-exclusive message." },
  {"readVariableLengthVal", (PyCFunction) PyMidiMessage_readVariableLengthVal, METH_VARARGS | METH_STATIC,
    "Reads a midi variable-length integer." },
  {"getMessageLengthFromFirstByte", (PyCFunction) PyMidiMessage_getMessageLengthFromFirstByte, METH_VARARGS | METH_STATIC,
    "Based on the first byte of a short midi message, this uses a lookup table to return the message length (either 1, 2, or 3 bytes)." },  
  
  {"getMidiNoteName", (PyCFunction) PyMidiMessage_getMidiNoteName, METH_VARARGS | METH_STATIC,
    "Returns the name of a midi note number. (E.g \"C\", \"D#\", etc.)"
    "noteNumber           the midi note number, 0 to 127\n"
    "useSharps            if true, sharpened notes are used, e.g. \"C#\", otherwise\n"
    "they'll be flattened, e.g. \"Db\"\n"
    "includeOctaveNumber  if true, the octave number will be appended to the string, e.g. \"C#4\"\n"
    "octaveNumForMiddleC  if an octave number is being appended, this indicates the\n"
    "number that will be used for middle C's octave." },
  {"getMidiNoteInHertz", (PyCFunction) PyMidiMessage_getMidiNoteInHertz, METH_VARARGS | METH_STATIC,
    "Returns the standard name of a GM instrument (instrument: 0 - 127)" },
  {"getGMInstrumentName", (PyCFunction) PyMidiMessage_getGMInstrumentName, METH_VARARGS | METH_STATIC,
    "Returns the name of a bank of GM instruments. (program: 0 - 127)" },
  {"getGMInstrumentBankName", (PyCFunction) PyMidiMessage_getGMInstrumentBankName, METH_VARARGS | METH_STATIC,
    "Returns the name of a bank of GM instruments. (bank: 0 - 15)" },
  {"getRhythmInstrumentName", (PyCFunction) PyMidiMessage_getRhythmInstrumentName, METH_VARARGS | METH_STATIC,
    "Returns the standard name of a channel 10 percussion sound. (key: 35 - 81)" },
  {"getControllerName", (PyCFunction) PyMidiMessage_getControllerName, METH_VARARGS | METH_STATIC,
    "Returns the name of a controller type number." },
  
  {0, 0, 0, 0}  /* Sentinel */
};


#if PK_PYTHON3
static PyTypeObject PyMidiMessage_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "rtmidi.MidiMessage", /* tp_name */
  sizeof(PyMidiMessage),     /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)PyMidiMessage_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  PyMidiMessage_str,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
  "midi message",            /* tp_doc */
  0,		                     /* tp_traverse */
  0,		                     /* tp_clear */
  (richcmpfunc)&PyMidiMessage___eq__,		   /* tp_richcompare */
  0,		                     /* tp_weaklistoffset */
  0,		                     /* tp_iter */
  0,		                     /* tp_iternext */
  PyMidiMessage_methods,     /* tp_methods */
  0,                         /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc) PyMidiMessage_init, /* tp_init */
  0,                         /* tp_alloc */
  PyMidiMessage_new,         /* tp_new */
};
#else
static PyTypeObject PyMidiMessage_Type = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "rtmidi.MidiMessage",  /*tp_name*/
  sizeof(PyMidiMessage),     /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor) PyMidiMessage_dealloc,    /*tp_dealloc*/
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
  PyMidiMessage_str,         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_RICHCOMPARE, /*tp_flags*/
  "midi message",            /* tp_doc */
  0,		                     /* tp_traverse */
  0,		                     /* tp_clear */
  PyMidiMessage___eq__,	     /* tp_richcompare */
  0,		                     /* tp_weaklistoffset */
  0,		                     /* tp_iter */
  0,		                     /* tp_iternext */
  PyMidiMessage_methods,     /* tp_methods */
  0, //PyMidiMessage_members,/* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc) PyMidiMessage_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyMidiMessage_new          /* tp_new */
};
#endif

PyTypeObject *getMidiMessageType()
{
  return &PyMidiMessage_Type;
}

bool PyMidiMessage_Check(PyObject *op)
{
  return PyObject_TypeCheck(op, &PyMidiMessage_Type);
}

PyObject *PyMidiMessage_new()
{
  return PyMidiMessage_new(getMidiMessageType(), 0, 0);
}


