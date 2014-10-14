import threading
import rtmidi
import random, time

class RandomOut(threading.Thread):
    def __init__(self, device=None):
        threading.Thread.__init__(self)
        self.device = device
        random.seed(time.time())

    def get(self):
        key = int(random.random() * 100)
        vel = int(random.random() * 100)
        msg = rtmidi.MidiMessage.noteOn(1, key, vel)
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
