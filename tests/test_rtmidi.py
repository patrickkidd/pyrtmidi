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

import os, sys

BUILD_PATHS = [os.path.join(os.getcwd(), 'build/lib.linux-x86_64-2.4/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.5.0-Power_Macintosh-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.8.1-i386-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.6-universal-2.6/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.9-x86_64-3.4'),
               ]
sys.path = BUILD_PATHS + sys.path

import unittest
import rtmidi


class RtMidiInTest(unittest.TestCase):
    def setUp(self):
        self.rtmidi = rtmidi.RtMidiIn()

    def test_01_inputs_exist(self):
        self.assertEqual(self.rtmidi.getPortCount() > 0, True)

    def test_funcs(self):
        """ test that certain functions can be called. """
        self.rtmidi.openVirtualPort('myport')
        self.rtmidi.ignoreTypes(False, True, False)
        self.rtmidi.closePort()
        for i in range(self.rtmidi.getPortCount()):
            self.rtmidi.getPortName(i)
        
    def test_blocking(self):
        """ test blocking reads. """
        self.rtmidi.openPort(0)
        print('move a MIDI device to send some data...')
        m1 = self.rtmidi.getMessage()
        m2 = self.rtmidi.getMessage()
        m3 = self.rtmidi.getMessage()
        self.rtmidi.closePort()

    def test_non_blocking(self):
        """ test non-blocking reads. """
        self.rtmidi.openPort(1)
        self.rtmidi.getMessage(0)
        self.rtmidi.getMessage(0)
        self.rtmidi.getMessage(0)
        self.rtmidi.closePort()
    
import time
class RtMidiOutTest(unittest.TestCase):
    def setUp(self):
        self.rtmidi = rtmidi.RtMidiOut()

    def test_00_openVirtual(self):
        vi = rtmidi.RtMidiOut()
        vi.openVirtualPort('myport')
        while 1:
#            print('sending ')
#            sys.stdout.flush()
            vi.sendMessage(0xF8)
            time.sleep(.016)
        vi.closePort()

    def test_01_outputs_exist(self):
        self.assertEqual(self.rtmidi.getPortCount() > 0, True)

    def test_funcs(self):
        """ test that certain functions can be called. """
        self.rtmidi.openPort(1)
        self.rtmidi.closePort()

        self.rtmidi.openVirtualPort()
        for i in range(self.rtmidi.getPortCount()):
            self.rtmidi.getPortName(i)




def print_ports(device):
    ports = range(device.getPortCount())
    if ports:
        for i in ports:
            print('   ', device.getPortName(i))
    else:
        print('NO MIDI PORTS!')


if __name__ == '__main__':
    print("MIDI IN PORTS:")
    print_ports(rtmidi.RtMidiIn())
    print("MIDI OUT PORTS:")
    print_ports(rtmidi.RtMidiOut())
    unittest.main()
