#!/usr/bin/env python3

import os, sys
sys.path.append(os.path.join(os.getcwd(), 'tests'))
try:
    import rtmidi_shim
except ImportError:
    pass
import rtmidi
#import rtmidi._rtmidi

def callback(collector, msg):
    print("%s: %s" % (collector.portName, msg))

collectors = rtmidi.CollectorBin()
collectors.start()
print('HIT ENTER TO EXIT')
try:
    sys.stdin.read(1)
except KeyboardInterrupt:
    pass
collectors.stop()
