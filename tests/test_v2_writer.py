import time, rtmidi
import randomout
from test_v2_reader import PORT_NAME

device = rtmidi.RtMidiOut()
print('OPENING %s' % PORT_NAME)
device.openVirtualPort(PORT_NAME)
randomOut = randomout.RandomOut(device)
try:
    randomOut.run()
except KeyboardInterrupt:
    pass
