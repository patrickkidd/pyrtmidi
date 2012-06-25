#! /usr/bin/python

import rtmidi
import sys

if len(sys.argv) != 3:
  print "Usage: rtmidi-sysex-test.py portnum sysex file"
  sys.exit(2)

portnum = int(sys.argv[1])
filename = sys.argv[2]

midiout = rtmidi.RtMidiOut()
midiout.openPort(portnum)

sysex = [ord(i) for i in open(filename, 'rb').read()]
#sysex = open(filename, 'rb').read()

midiout.sendMessage(*sysex)

