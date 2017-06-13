pyrtmidi
========

Realtime MIDI I/O for Python on Windows, OS X, and Linux. Includes comprehensive MidiMessage class, support for virtual ports on OS X and Linux, and multi-threaded lister utility classes.

Pyrtmidi provides MIDI I/O for [PKMidiCron](http://vedanamedia.com/our-products/pkmidicron/).

Usage
-------------

pyrtmidi is a Python interface to RtMidi. It provides real-time midi input and output.

```python

def print_message(midi):
    if midi.isNoteOn():
        print 'ON: ', midi.getMidiNoteName(midi.getNoteNumber()), midi.getVelocity()
    elif midi.isNoteOff():
        print 'OFF:', midi.getMidiNoteName(midi.getNoteNumber())
    elif midi.isController():
        print 'CONTROLLER', midi.getControllerNumber(), midi.getControllerValue()


import rtmidi
midiin = rtmidi.RtMidiIn()

ports = range(midiin.getPortCount())
if ports:
    for i in ports:
        print midiin.getPortName(i)
    midiin.openPort(1)
    while True:
        m = midiin.getMessage(250) # some timeout in ms
        if m != None:
            print_message(m)
else:
    print 'NO MIDI INPUT PORTS!'
```

The API is copied *near* verbatim from the C++ code. Refer to the [RtMidi tutorial](http://www.music.mcgill.ca/~gary/rtmidi/), and take into account the following caveats:

*RtMidiIn*

`getMessage(timeout_ms=None)`

 * The message argument has been replaced with an optional millisecond timeout value.
 * The function will return a MidiMessage object, or None if no message is available.

`setCallback`

 * This function works just as described in the above docs, and takes a python callable object.
 * This method is most useful with a queue.Queue object to communicate between threads.

*MidiMessage*

This class has been taken from the juce library, and includes an excellent comprehensive set of midi functions. [please check here for available methods](https://www.juce.com/doc/classMidiMessage).

*Recipes*

The following implements a QObject wrapper for rtmidi.RtMidiIn that emits a 'message()' signal. The essential code is follows:

```python
class MidiInput(QThread):
    def __init__(self, devid, parent=None):
        QThread.__init__(self, parent)
        self.device = rtmidi.RtMidiIn()
        self.device.openPort(devid)
        self.running = False

    def run(self):
        self.running = True
        while self.running:
            msg = self.device.getMessage(250)
            if msg:
                self.msg = msg
                self.emit(SIGNAL('message(PyObject *)'), self.msg)
                self.emit(SIGNAL('message()')

midi = MidiInput(1)
def slotMessage(msg):
   print msg
QObject.connect(midi, SIGNAL('message(PyObject *)'), slotMessage)
```

The following implements a midi echo for all ports.

```python

import sys
import rtmidi
import threading

def print_message(midi, port):
    if midi.isNoteOn():
        print '%s: ON: ' % port, midi.getMidiNoteName(midi.getNoteNumber()), midi.getVelocity()
    elif midi.isNoteOff():
        print '%s: OFF:' % port, midi.getMidiNoteName(midi.getNoteNumber())
    elif midi.isController():
        print '%s: CONTROLLER' % port, midi.getControllerNumber(), midi.getControllerValue()

class Collector(threading.Thread):
    def __init__(self, device, port):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.port = port
        self.portName = device.getPortName(port)
        self.device = device
        self.quit = False

    def run(self):
        self.device.openPort(self.port)
        self.device.ignoreTypes(True, False, True)
        while True:
            if self.quit:
                return
            msg = self.device.getMessage()
            if msg:
                print_message(msg, self.portName)


dev = rtmidi.RtMidiIn()
collectors = []
for i in range(dev.getPortCount()):
    device = rtmidi.RtMidiIn()
    print 'OPENING',dev.getPortName(i)
    collector = Collector(device, i)
    collector.start()
    collectors.append(collector)


print 'HIT ENTER TO EXIT'
sys.stdin.read(1)
for c in collectors:
    c.quit = True
```




Common Problems
-----------------

This is a commonly reported build error on linux, although I it works for me using python2.7 on ubuntu.

```
:~/devel/pkaudio/pyrtmidi/tests$ python test_rtmidi.py 
Traceback (most recent call last):
File "test_rtmidi.py", line 29, in 
import rtmidi
ImportError: /usr/local/lib/python2.6/dist-packages/rtmidi.so: undefined symbol: _ZN11MidiMessageaSERKS_
```
