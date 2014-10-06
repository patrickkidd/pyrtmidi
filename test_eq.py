import unittest
from rtmidi import MidiMessage


class TestEQ(unittest.TestCase):
    def setUp(self):
        pass
    def test_eq(self):
        self.assertTrue(MidiMessage.noteOn(1, 0, 100) == MidiMessage.noteOn(1, 0, 100))
        self.assertFalse(MidiMessage.noteOn(1, 1, 100) == MidiMessage.noteOn(1, 0, 100))

        self.assertFalse(MidiMessage.pitchWheel(1, 1) == MidiMessage.noteOn(1, 64, 1)) 

        self.assertTrue(MidiMessage.controllerEvent(1, 100, 1) ==
                         MidiMessage.controllerEvent(1, 100, 1))
        self.assertTrue(MidiMessage.controllerEvent(1, 100, 2) ==
                         MidiMessage.controllerEvent(1, 100, 2))
        self.assertTrue(MidiMessage.controllerEvent(1, 100, 3) ==
                         MidiMessage.controllerEvent(1, 100, 3))

        self.assertFalse(MidiMessage.controllerEvent(1, 101, 1) ==
                         MidiMessage.controllerEvent(1, 100, 1))
        self.assertFalse(MidiMessage.controllerEvent(1, 102, 2) ==
                         MidiMessage.controllerEvent(1, 100, 2))
        self.assertFalse(MidiMessage.controllerEvent(1, 103, 3) ==
                         MidiMessage.controllerEvent(1, 100, 3))


if __name__ == '__main__':
    unittest.main()
