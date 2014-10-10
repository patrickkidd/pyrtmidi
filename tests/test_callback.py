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


import os, sys

BUILD_PATHS = [os.path.join(os.getcwd(), 'build/lib.linux-x86_64-2.4/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.5.0-Power_Macintosh-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.8.1-i386-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.11.0-Power_Macintosh-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.6-universal-2.6/'),
               ]
sys.path = BUILD_PATHS + sys.path

import imp
bleh = imp.find_module('rtmidi')
print 'Using rtmidi module at:', bleh[1]
import rtmidi
import time

def print_ports(device):
    ports = range(device.getPortCount())
    if ports:
        for i in ports:
            print 'MIDI IN:',device.getPortName(i)
    else:
        print 'NO MIDI PORTS!'


def print_message(midi):
    if midi.isNoteOn():
        print 'ON: ', midi.getMidiNoteName(midi.getNoteNumber()), midi.getVelocity()
    elif midi.isNoteOff():
        print 'OFF:', midi.getMidiNoteName(midi.getNoteNumber())
    elif midi.isController():
        print 'CONTROLLER', midi.getControllerNumber(), midi.getControllerValue()
    elif midi.isSysEx():
        print 'SYSEX (%i bytes)' % midi.getSysExData()
    elif midi.isAftertouch():
        print 'AFTERTOUCH: ', midi.getAfterTouchValue()

midiout = None
def callback(midi):
    global midiout
    print_message(midi)
    if midiout:
        midiout.sendMessage(midi)



def main_in():
    midiin = rtmidi.RtMidiIn()
    print_ports(midiin)
    
    midiin.setCallback(callback)
    midiin.openPort(1) # KeyStation?

    try:
        while True:
            time.sleep(.1)
    except KeyboardInterrupt:
        pass
    midiin.closePort()
    print 'quit'


def main_in_block():
    midiin = rtmidi.RtMidiIn()
    print_ports(midiin)
    
    midiin.openPort(1) # KeyStation?

    try:
        m = None
        while True:
            m = midiin.getMessage(250)
            if not m is None:
                print_message(m)
    except KeyboardInterrupt:
        pass
    midiin.closePort()
    print 'quit'


def main_in_noblock():
    midiin = rtmidi.RtMidiIn()
    print_ports(midiin)
    
    midiin.openPort(1) # KeyStation?

    try:
        m = None
        while True:
            m = midiin.getMessage()
            if not m is None:
                print_message(m)
    except KeyboardInterrupt:
        pass
    midiin.closePort()
    print 'quit'

def main_inout():
    global midiout
    midiin = rtmidi.RtMidiIn()
    print 'INPUTS:'
    print_ports(midiin)
    midiin.setCallback(callback)
    midiin.openPort(1) # KeyStation?
    
    midiout = rtmidi.RtMidiOut()
    print 'OUTPUTS:'
    print_ports(midiout)
    midiout.openPort(0) # IAC?

    while True:
        time.sleep(.1)
    midiin.closePort()
    print 'quit'


def main_inout_block():
    # don't set global var because we will output here
    # global midiout
    
    midiin = rtmidi.RtMidiIn()
    print 'INPUTS:'
    print_ports(midiin)
    midiin.openPort(1) # KeyStation?
    
    midiout = rtmidi.RtMidiOut()
    print 'OUTPUTS:'
    print_ports(midiout)
    midiout.openPort(0) # IAC?

    try:
        m = None
        while True:
            m = midiin.getMessage(250)
            if not m is None:
                midiout.sendMessage(m)
                m = rtmidi.MidiMessage.aftertouchChange(1, 1, m.getNoteNumber())
                print_message(m)
    except KeyboardInterrupt:
        pass
        
    midiin.closePort()
    print 'quit'
    
    
if __name__ == '__main__':
    #main_in()
    #main_in_block()
    #main_inout()
    main_inout_block()
    #main_in_noblock()
