#!/usr/bin/env python

import os, sys
_BUILD_PATHS = [
    os.path.join(os.getcwd(), 'build/lib.linux-x86_64-2.4/'),
    os.path.join(os.getcwd(), 'build/lib.darwin-8.5.0-Power_Macintosh-2.3/'),
    os.path.join(os.getcwd(), 'build/lib.macosx-10.9-x86_64-3.4'),
]
BUILD_PATHS = []
for i in _BUILD_PATHS:
    if os.path.exists(i):
        BUILD_PATHS.append(i)
sys.path = BUILD_PATHS + sys.path

import imp
print('LOADING MODULE: %s' % imp.find_module('rtmidi')[1])
import rtmidi
import threading

def print_message(midi, port):
    if midi.isNoteOn():
        print('%s: ON: ' % port, midi.getMidiNoteName(midi.getNoteNumber()), midi.getVelocity())
    elif midi.isNoteOff():
        print('%s: OFF:' % port, midi.getMidiNoteName(midi.getNoteNumber()))
    elif midi.isController():
        print('%s: CONTROLLER' % port, midi.getControllerNumber(), midi.getControllerValue())

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
            msg = self.device.getMessage(250)
            if msg:
                print_message(msg, self.portName)


dev = rtmidi.RtMidiIn()
collectors = []
for i in range(dev.getPortCount()):
    device = rtmidi.RtMidiIn()
    print('OPENING',dev.getPortName(i))
    collector = Collector(device, i)
    collector.start()
    collectors.append(collector)


print('HIT ENTER TO EXIT')
sys.stdin.read(1)
for c in collectors:
    c.quit = True
