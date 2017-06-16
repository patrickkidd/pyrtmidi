import threading
import rtmidi
import random, time

class RandomOut(threading.Thread):
    def __init__(self, device=None):
        threading.Thread.__init__(self)
        self.device = device
        random.seed(time.time())

    def get(self):
        a = int(random.random() * 128)
        b = int(random.random() * 128)
        c = int(random.random() * 10)
        if c == 0:
            msg = rtmidi.MidiMessage.noteOn(1, a, b)
        elif c == 1:
            msg = rtmidi.MidiMessage.noteOff(1, a)
        elif c == 2:
            msg = rtmidi.MidiMessage.controllerEvent(1, a, b)
        elif c == 3:
            msg = rtmidi.MidiMessage.aftertouchChange(1, a, b)
        elif c == 4:
            msg = rtmidi.MidiMessage.channelPressureChange(1, a)
        elif c == 5:
            msg = rtmidi.MidiMessage.programChange(1, a)
        elif c == 6:
            msg = rtmidi.MidiMessage.pitchWheel(1, a)
        elif c == 7:
            msg = rtmidi.MidiMessage.allNotesOff(1)
        elif c == 8:
            msg = rtmidi.MidiMessage.allSoundOff(1)
        elif c == 9:
            msg = rtmidi.MidiMessage.allControllersOff(1)
        return msg

    def run(self):
        if self.device is None:
            return
        random.seed(time.time())
        while True:
            print('SENDING: %s' % msg)
            msg = self.get()
            self.device.sendMessage(msg)
            ts = random.random() * 2
            time.sleep(ts)
