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

import os, sys, imp, time

BUILD_PATHS = [os.path.join(os.getcwd(), 'build/lib.linux-x86_64-2.4/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.5.0-Power_Macintosh-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.8.1-i386-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.6-universal-2.6/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.9-x86_64-3.4'),
               ]
sys.path = BUILD_PATHS + sys.path
bleh = imp.find_module('rtmidi')
print('%s: Using rtmidi module at:' % __name__, bleh[1])

import unittest
import rtmidi


def SenderProc(iq, oq, portName):
    DEBUG = 0
    def wait_for(x):
        s = iq.get()
        if s != x:
            print("%s: OH SHIT (wait_for() %s != %s)" % (__name__, s, x))

    device = rtmidi.RtMidiOut()
    if DEBUG: print('%s: OPENING %s' % (__name__, portName))
    device.openVirtualPort(portName)

    oq.put('init') # the port is open

    wait_for('start')
    total = 0
    # note on
    for i in range(128):
        for j in range(1, 128):
            if DEBUG: print("%s: Note %i %i" % (__name__, i, j))
            m = rtmidi.MidiMessage.noteOn(1, i, j)
            device.sendMessage(m)
            wait_for('next')
            total += 1
    # controller
    for i in range(128):
        for j in range(128):
            if DEBUG: print("%s: CC %i %i" % (__name__, i, j))
            m = rtmidi.MidiMessage.controllerEvent(1, i, j)
            device.sendMessage(m)
            wait_for('next')
            total += 1

    wait_for('done')
    print('%s: sent %i messages' % (__name__, total))


class TransmissionTest(unittest.TestCase):

    def setUp(self):
        pass

    def test_transmission(self):
        import multiprocessing as mp
        mp.set_start_method('spawn')

        iq = mp.Queue()
        oq = mp.Queue()
        portName = 'rtmidi.TestVirtualPorts.%i' % time.time()
        senderProc = mp.Process(target=SenderProc, args=(oq, iq, portName))
        senderProc.start()

        # handshake
        self.assertEqual(iq.get(), 'init') # virtual midi port is now open

        # Supposedly you can't just open a virtual port by name from
        # within the same proc or proc group? Anyway opening by index
        # works.
        device = rtmidi.RtMidiIn()
        for i in range(device.getPortCount()):
            if device.getPortName(i) == portName:
                device.openPort(i)
                break        
        self.assertTrue(device.isPortOpen())

        # collect messages and print progress
        self.messages = []
        self.last_s = ''
        def put(m):
            self.messages.append(m)
            if len(self.messages) % 10 == 0:
                sys.stdout.write('\b' * len(self.last_s)) # backspace
                self.last_s = '%s: Received message %i / 32640' % (__name__, len(self.messages))
                sys.stdout.write(self.last_s)
                # sys.stdout.write('.')
                sys.stdout.flush()

        oq.put('start')
        for i in range(128):
            for j in range(1, 128):
                msg = device.getMessage(1000)
                self.assertIsNotNone(msg)
                self.assertTrue(msg.isNoteOn())
                self.assertEqual(msg.getNoteNumber(), i)
                self.assertEqual(msg.getVelocity(), j)
                put(msg)
                oq.put('next')
                
        for i in range(128):
            for j in range(128):
                msg = device.getMessage(1000)
                self.assertIsNotNone(msg)
                self.assertTrue(msg.isController())
                self.assertEqual(msg.getControllerNumber(), i)
                self.assertEqual(msg.getControllerValue(), j)
                put(msg)
                oq.put('next')

        self.assertEqual(len(self.messages), 32640)
        oq.put('done')
        





def print_ports(device):
    ports = range(device.getPortCount())
    if ports:
        for i in ports:
            print('   ', device.getPortName(i))
    else:
        print('NO MIDI PORTS!')


if __name__ == '__main__':
#    print("MIDI IN PORTS:")
#    print_ports(rtmidi.RtMidiIn())
#    print("MIDI OUT PORTS:")
#    print_ports(rtmidi.RtMidiOut())
    unittest.main()
