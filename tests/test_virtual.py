import rtmidi
import random, time
import randomout
import sys

PORT_NAME = 'rtmidi.TestVirtualPorts.%i' % time.time()
def SenderProc(iq, oq, portName):
    device = rtmidi.RtMidiOut()
    print('SenderProc: OPENING %s' % PORT_NAME)
    device.openVirtualPort(PORT_NAME)
    DEBUG = 0

    oq.put('start')
    if iq.get() != 'ok':
        print("%s: OH SHIT 1!" % __name__)

    spacing = .0001
    total = 0
    j = 1
    for i in range(128):
        for j in range(1, 128):
            if DEBUG: print("SenderProc: Note %i %i" % (i, j))
            m = rtmidi.MidiMessage.noteOn(1, i, j)
            device.sendMessage(m)
            total += 1
            time.sleep(spacing)
    for i in range(128):
        for j in range(128):
            if DEBUG: print("SenderProc: CC %i %i" % (i, j))
            m = rtmidi.MidiMessage.controllerEvent(1, i, j)
            device.sendMessage(m)
            total += 1
            time.sleep(spacing)

    print("\nSenderProc: sent (%i) messages, waiting for 'done'" % total)
    if iq.get() != 'done':
        return "SenderProc: OH SHIT 2!"

    print('SenderProc: done')



if __name__ == '__main__':
    import multiprocessing as mp
    mp.set_start_method('spawn')
    iq = mp.Queue()
    oq = mp.Queue()
    senderProc = mp.Process(target=SenderProc, args=(oq, iq, PORT_NAME))
    senderProc.start()

    
    if iq.get() != 'start': # port is now open
        print("%s: OH SHIT 1!" % __name__)
    

    qDevice = rtmidi.RtMidiIn(rtmidi.RtMidiIn.UNSPECIFIED, "Something", 100)
    for i in range(qDevice.getPortCount()):
        print('FOUND: ', qDevice.getPortName(i))


    oq.put('ok')
    device = rtmidi.RtMidiIn()
    # supposedly you can't just open a virtual port by name from
    # within the same proc or proc group? Anyway opening by index
    # works.
    for i in range(device.getPortCount()):
        if device.getPortName(i) == PORT_NAME:
            print("%s: OPENING %s" % (__name__, device.getPortName(i)))
            device.openPort(i)
            break;
    oq.put('start')

    messages = []
    last_s = ''
    def put(m):
        global last_s
        messages.append(m)
        if len(messages) % 10 == 0:
            sys.stdout.write('\b' * len(last_s))
            last_s = 'message %i / 32640' % len(messages)
            sys.stdout.write(last_s)
#            sys.stdout.write('.')
            sys.stdout.flush()

    j = 1
    for i in range(128):
        for j in range(1, 128):
            msg = device.getMessage(1000)
            if msg is None:
                print("%s: OH SHIT 6! (%i)" % (__name__, len(messages)))
                sys.exit(1)
            elif not msg.isNoteOn() or not msg.getNoteNumber() == i or not msg.getVelocity() == j:
                print("%s: OH SHIT4! Note (%i, %i) %s" % (__name__, i, j, msg))
            else:
                put(msg)
            
    for i in range(128):
        for j in range(128):
            msg = device.getMessage(1000)
            if msg is None:
                print("%s: OH SHIT 7! (%i)" % (__name__, len(messages)))
                sys.exit(1)
            elif not msg.isController() or not msg.getControllerNumber() == i or not msg.getControllerValue() == j:
                print("%s: OH SHIT4! CC (%i, %i) %s" % (__name__, i, j, msg))
            else:
                put(msg)

    oq.put('done')
