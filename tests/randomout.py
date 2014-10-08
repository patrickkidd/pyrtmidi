import threading
import rtmidi
import random, time

class RandomOut(threading.Thread):
    def __init__(self, device):
        threading.Thread.__init__(self)
        self.device = device

    def run(self):
        random.seed(time.time())
        while True:
            key = int(random.random() * 100)
            vel = int(random.random() * 100)
            msg = rtmidi.MidiMessage.noteOn(1, key, vel)
            print('SENDING: %s' % msg)
            self.device.sendMessage(msg)
            ts = random.random() * 2
            time.sleep(ts)
