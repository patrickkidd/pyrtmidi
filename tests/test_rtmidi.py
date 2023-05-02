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
import time
import unittest
from rtmidi import MidiMessage, RtMidiOut, RtMidiIn


class TestMidimessage(unittest.TestCase):
    def test_noteOnChannel(self):
        """https://github.com/patrickkidd/pyrtmidi/issues/13"""
        m = MidiMessage.noteOn(10, 12, 13)
        self.assertEqual(m.getChannel(), 10)
        self.assertEqual(m.getNoteNumber(), 12)
        self.assertEqual(m.getVelocity(), 13)

    def test_statics(self):
        ctls = [
            "Bank Select",
            "Modulation Wheel (coarse)",
            "Breath controller (coarse)",
            "",
            "Foot Pedal (coarse)",
            "Portamento Time (coarse)",
            "Data Entry (coarse)",
            "Volume (coarse)",
            "Balance (coarse)",
            "",
            "Pan position (coarse)",
            "Expression (coarse)",
            "Effect Control 1 (coarse)",
            "Effect Control 2 (coarse)",
            "",
            "",
            "General Purpose Slider 1",
            "General Purpose Slider 2",
            "General Purpose Slider 3",
            "General Purpose Slider 4",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "Bank Select (fine)",
            "Modulation Wheel (fine)",
            "Breath controller (fine)",
            "",
            "Foot Pedal (fine)",
            "Portamento Time (fine)",
            "Data Entry (fine)",
            "Volume (fine)",
            "Balance (fine)",
            "",
            "Pan position (fine)",
            "Expression (fine)",
            "Effect Control 1 (fine)",
            "Effect Control 2 (fine)",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "Hold Pedal (on/off)",
            "Portamento (on/off)",
            "Sustenuto Pedal (on/off)",
            "Soft Pedal (on/off)",
            "Legato Pedal (on/off)",
            "Hold 2 Pedal (on/off)",
            "Sound Variation",
            "Sound Timbre",
            "Sound Release Time",
            "Sound Attack Time",
            "Sound Brightness",
            "Sound Control 6",
            "Sound Control 7",
            "Sound Control 8",
            "Sound Control 9",
            "Sound Control 10",
            "General Purpose Button 1 (on/off)",
            "General Purpose Button 2 (on/off)",
            "General Purpose Button 3 (on/off)",
            "General Purpose Button 4 (on/off)",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "Reverb Level",
            "Tremolo Level",
            "Chorus Level",
            "Celeste Level",
            "Phaser Level",
            "Data Button increment",
            "Data Button decrement",
            "Non-registered Parameter (fine)",
            "Non-registered Parameter (coarse)",
            "Registered Parameter (fine)",
            "Registered Parameter (coarse)",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "All Sound Off",
            "All Controllers Off",
            "Local Keyboard (on/off)",
            "All Notes Off",
            "Omni Mode Off",
            "Omni Mode On",
            "Mono Operation",
            "Poly Operation",
        ]
        for i, v in enumerate(ctls):
            self.assertEqual(MidiMessage.getControllerName(i), v)
        # crash test
        self.assertEqual(MidiMessage.getControllerName(-1), "")
        self.assertEqual(MidiMessage.getControllerName(128), "")

    def test_operators(self):
        m1 = MidiMessage.noteOn(1, 100, 120)
        m2 = MidiMessage.noteOn(1, 100, 120)
        self.assertEqual(m1, m2)

        m3 = MidiMessage.noteOn(1, 101, 120)
        self.assertNotEqual(m1, m3)

    def test_eq(self):
        self.assertTrue(MidiMessage.noteOn(1, 0, 100) == MidiMessage.noteOn(1, 0, 100))
        self.assertFalse(MidiMessage.noteOn(1, 1, 100) == MidiMessage.noteOn(1, 0, 100))

        self.assertFalse(MidiMessage.pitchWheel(1, 1) == MidiMessage.noteOn(1, 64, 1))

        self.assertTrue(
            MidiMessage.controllerEvent(1, 100, 1)
            == MidiMessage.controllerEvent(1, 100, 1)
        )
        self.assertTrue(
            MidiMessage.controllerEvent(1, 100, 2)
            == MidiMessage.controllerEvent(1, 100, 2)
        )
        self.assertTrue(
            MidiMessage.controllerEvent(1, 100, 3)
            == MidiMessage.controllerEvent(1, 100, 3)
        )

        self.assertFalse(
            MidiMessage.controllerEvent(1, 101, 1)
            == MidiMessage.controllerEvent(1, 100, 1)
        )
        self.assertFalse(
            MidiMessage.controllerEvent(1, 102, 2)
            == MidiMessage.controllerEvent(1, 100, 2)
        )
        self.assertFalse(
            MidiMessage.controllerEvent(1, 103, 3)
            == MidiMessage.controllerEvent(1, 100, 3)
        )

    def test_copy(self):
        m1 = MidiMessage.noteOn(5, 123, 45)
        m2 = MidiMessage(m1)
        self.assertEqual(m1, m2)
        self.assertEqual(m1.getChannel(), m2.getChannel())

    def test_raw(self):
        m1 = MidiMessage.noteOn(5, 123, 45)
        data = m1.getRawData()
        size = m1.getRawDataSize()
        self.assertEqual(size, len(data))
        m2 = MidiMessage(data)
        self.assertEqual(m1, m2)
        #
        m1 = MidiMessage.controllerEvent(12, 123, 45)
        data = m1.getRawData()
        size = m1.getRawDataSize()
        self.assertEqual(size, len(data))
        m2 = MidiMessage(data)
        self.assertEqual(m1, m2)


def SenderProc(iq, oq, portName):
    try:
        do_SenderProc(iq, oq, portName)
    except:
        pass


def do_SenderProc(iq, oq, portName):
    DEBUG = 0

    def wait_for(x):
        s = iq.get(block=True, timeout=2)
        if s != x:
            print("%s: OH SHIT (wait_for() %s != %s)" % (__name__, s, x))

    device = RtMidiOut()
    if DEBUG:
        print("%s: OPENING %s" % (__name__, portName))
    device.openVirtualPort(portName)

    oq.put("init")  # the port is open

    wait_for("start")
    total = 0
    # note on
    for i in range(128):
        for j in range(1, 128):
            if DEBUG:
                print("%s: Note %i %i" % (__name__, i, j))
            m = MidiMessage.noteOn(1, i, j)
            device.sendMessage(m)
            wait_for("next")
            total += 1
    # controller
    for i in range(128):
        for j in range(128):
            if DEBUG:
                print("%s: CC %i %i" % (__name__, i, j))
            m = MidiMessage.controllerEvent(1, i, j)
            device.sendMessage(m)
            wait_for("next")
            total += 1

    wait_for("done")
    print("%s: sent %i messages" % (__name__, total))


class TransmissionTest(unittest.TestCase):
    def setUp(self):
        pass

    def test_transmission(self):
        import multiprocessing as mp

        if hasattr(mp, "set_start_method"):
            mp.set_start_method("spawn")

        iq = mp.Queue()
        oq = mp.Queue()
        portName = "TestVirtualPorts.%i" % time.time()
        senderProc = mp.Process(target=SenderProc, args=(oq, iq, portName))
        senderProc.start()

        # handshake
        self.assertEqual(iq.get(), "init")  # virtual midi port is now open

        # Supposedly you can't just open a virtual port by name from
        # within the same proc or proc group? Anyway opening by index
        # works.
        device = RtMidiIn()
        for i in range(device.getPortCount()):
            if device.getPortName(i) == portName:
                device.openPort(i)
                break
        self.assertTrue(device.isPortOpen())

        # collect messages and print progress
        self.messages = []
        self.last_s = ""

        def put(m):
            self.messages.append(m)
            if len(self.messages) % 10 == 0:
                sys.stdout.write("\b" * len(self.last_s))  # backspace
                self.last_s = "%s: Received message %i / 32640" % (
                    __name__,
                    len(self.messages),
                )
                sys.stdout.write(self.last_s)
                # sys.stdout.write('.')
                sys.stdout.flush()

        oq.put("start")
        for i in range(128):
            for j in range(1, 128):
                msg = device.getMessage(1000)
                self.assertTrue(msg is not None)
                self.assertTrue(msg.isNoteOn())
                self.assertEqual(msg.getNoteNumber(), i)
                self.assertEqual(msg.getVelocity(), j)
                put(msg)
                oq.put("next")

        for i in range(128):
            for j in range(128):
                msg = device.getMessage(1000)
                self.assertTrue(msg is not None)
                self.assertTrue(msg.isController())
                self.assertEqual(msg.getControllerNumber(), i)
                self.assertEqual(msg.getControllerValue(), j)
                put(msg)
                oq.put("next")

        self.assertEqual(len(self.messages), 32640)
        oq.put("done")


if os.name == "nt":
    TransmissionTest = None


def print_ports(device):
    ports = range(device.getPortCount())
    if ports:
        for i in ports:
            print("   ", device.getPortName(i))
    else:
        print("NO MIDI PORTS!")


if __name__ == "__main__":
    # print("MIDI IN PORTS:")
    # print_ports(RtMidiIn())
    # print("MIDI OUT PORTS:")
    # print_ports(RtMidiOut())
    unittest.main()
