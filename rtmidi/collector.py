import rtmidi
import threading

class Collector(threading.Thread):
    def __init__(self, device, callback):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.device = device
        self.callback = callback
        self.device.ignoreTypes(True, False, True)
        self.running = False

    def run(self):
        if rtmidi.DEBUG: print("rtmidi.Collector.start: " + self.portName)
        while self.running:
            msg = self.device.getMessage(100)
            if msg:
                self.callback(self, msg)
        if rtmidi.DEBUG: print("rtmidi.Collector.exiting: " + self.portName)
        self.running = False

    def start(self):
        self.running = True
        super().start()

    def stop(self):
        if rtmidi.DEBUG: print("rtmidi.Collector.stop: " + self.portName)
        self.running = False
        self.join()


class CollectorBin(threading.Thread):

    _bin = []

    def __init__(self, callback=None, autolist=True):
        threading.Thread.__init__(self)
        self._callback = callback and callback or self._callback
        self.collectors = {}
        if autolist:
            if rtmidi.DEBUG: print("rtmidi.CollectorBin.autolist = True")
            device = rtmidi.RtMidiIn()
            for i in range(device.getPortCount()):
                self.addCollector(device.getPortName(i))
        self.cond = threading.Condition()
        CollectorBin._bin.append(self)
        self.running = False

    def addCollector(self, portName, start=False):
        if portName in self.collectors:
            return
        device = rtmidi.RtMidiIn()
        iPort = None
        for i in range(device.getPortCount()):
            if device.getPortName(i) == portName:
                iPort = i
                break
        if iPort is None:
            return
        if rtmidi.DEBUG: print('rtmidi.Collector.addCollector', portName, iPort)
        try:
            device.openPort(i)
        except:
            return
        collector = Collector(device, self._callback)
        collector.portName = portName
        self.collectors[portName] = {
            'collector': collector,
            'name': portName,
            'queue': []
        }
        if start:
            collector.start()
        return collector

    def removeCollector(self, portName):
        """ stops collector. """
        if not portName in self.collectors:
            return
        if rtmidi.DEBUG: print('rtmidi.Collector.removeCollector', portName)
        c = self.collectors[portName]['collector']
        c.stop()
        c.join()
        del self.collectors[portName]

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
            if rtmidi.DEBUG: print("rtmidi.CollectorBin: running")
            self.running = True
            while self.running:
                with self.cond:
                    self.cond.wait_for(self._is_item_available, .1)
                    for k, v in self.collectors.items():
                        q = v['queue']
                        while len(q):
                            msg = q.pop(0)
                            print('%s: %s' % (k, msg))
            if rtmidi.DEBUG: print("rtmidi.CollectorBin: exiting....")
        except KeyboardInterrupt:
#            print("EXITING...")
            pass
        self.running = False

    def start(self):
        if self.running:
            return
        threading.Thread.start(self)
        for k, v in self.collectors.items():
            v['collector'].start()

    def stop(self):
        self.running = False
        if self.is_alive():
            self.join()
        for k, v in self.collectors.items():
            v['collector'].stop()
        self.joinAll()

    def joinAll(self):
        for k, v in self.collectors.items():
            c = v['collector']
            if c.is_alive():
                c.join()

    def names(self):
        """ return a list of open ports. """
        return list(self.collectors.keys())

    @staticmethod
    def cleanup():
        for c in CollectorBin._bin:
            c.stop()
            CollectorBin._bin.remove(c)
