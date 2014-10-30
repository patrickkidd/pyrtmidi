import rtmidi
import threading

class Collector(threading.Thread):
    def __init__(self, device, callback):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.device = device
        self.callback = callback
        self.device.ignoreTypes(True, False, True)

    def run(self):
        self.running = True
#        print("Collector.start: " + self.portName)
        while self.running:
            msg = self.device.getMessage(100)
            if msg:
                self.callback(self, msg)
#        print("Collector.exiting: " + self.portName)

    def stop(self):
#        print("Collector.stop: " + self.portName)
        self.running = False


class CollectorBin(threading.Thread):

    _bin = []

    def __init__(self, callback=None):
        threading.Thread.__init__(self)
        self.collectors = {}
        for i in range(rtmidi.RtMidiIn().getPortCount()):
            device = rtmidi.RtMidiIn()
            portName = device.getPortName(i)
#            print('OPENING', portName)
            device.openPort(i)
            collector = Collector(device, callback and callback or self._callback)
            collector.portName = portName
            self.collectors[portName] = {
                'collector': collector,
                'name': portName,
                'queue': []
            }
        self.cond = threading.Condition()
        CollectorBin._bin.append(self)

    # not thread safe!
    def _is_item_available(self):
        count = 0
        for k, v in self.collectors.items():
            count += len(v['queue'])
        return count > 0

    def _callback(self, collector, msg):
        with self.cond:
            self.collectors[collector.portName]['queue'].append(msg)
            self.cond.notify()

    def run(self):        
        try:
            self.running = True
            while self.running:
                with self.cond:
                    self.cond.wait_for(self._is_item_available, .1)
                    for k, v in self.collectors.items():
                        q = v['queue']
                        while len(q):
                            msg = q.pop(0)
                            print('%s: %s' % (k, msg))

        except KeyboardInterrupt:
#            print("EXITING...")
            return

    def start(self):
        threading.Thread.start(self)
        for k, v in self.collectors.items():
            v['collector'].start()

    def stop(self):
        self.running = False
        self.join()
        for k, v in self.collectors.items():
            v['collector'].stop()
        self.joinAll()

    def joinAll(self):
        for k, v in self.collectors.items():
            v['collector'].join()

    @staticmethod
    def cleanup():
        for c in CollectorBin._bin:
            c.stop()
            CollectorBin._bin.remove(c)            

