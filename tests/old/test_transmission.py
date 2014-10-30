###############################################################################
#
#
#      This scripts checks correct sending and receiving (in a separate thread)
#      of the various MIDI message kinds.
#      
#      Output looks like this:
#
#      Correct message type: True                                        <-- Message has been successfully checked with the respective is*()-function, where available.
#      <NOTE ON, C-5 (note 0), velocity: 127, channel: 2> (transmitted)  <-- Printed message before it has been transmitted.
#      <NOTE ON, C-5 (note 0), velocity: 127, channel: 2> (received)     <-- Printed message after it has been received.
#                                                                            If the two lines are identical, the transmission was successful.


import rtmidi, time



# Transmitter
midiout = rtmidi.RtMidiOut()
available_ports = midiout.getPortCount()
midiout.openVirtualPort("pyrtmidi unit test virtual midi port")


# Receiver
import threading
class MidiReceiver(threading.Thread):
        def __init__(self): 
                threading.Thread.__init__(self) 
                self.running = True
 
	def run(self): 
		print('Receiver thread started...')

		# Setup Receiver
		import rtmidi
		self.midiin = rtmidi.RtMidiIn()
		ports = range(self.midiin.getPortCount())
		if ports:
			for i in ports:
				print('Receiving from midi port "%s"' % self.midiin.getPortName(i))
			self.midiin.openPort(len(ports)-1)
			while self.running:
				message = self.midiin.getMessage(250) # some timeout in ms
				if message != None:
					print(message, '(received)')
		else:
			print('NO MIDI INPUT PORTS!')

	def stopReceiving(self):
		self.midiin.closePort()
		self.running = False
		

receiver = MidiReceiver()
receiver.start()


# Transmitting
time.sleep(0.1)
print()
print('Now transmitting messages...')
3,

# Send messages
m = rtmidi.MidiMessage()
messages = (
	('isAftertouch', rtmidi.MidiMessage().aftertouchChange(1, 10, 100)),
	('isAftertouch', rtmidi.MidiMessage().aftertouchChange(2, 20, 127)),
	(None, rtmidi.MidiMessage().allControllersOff(1)),
	(None, rtmidi.MidiMessage().allControllersOff(2)),
	('isAllNotesOff', rtmidi.MidiMessage().allNotesOff(1)),
	('isAllNotesOff', rtmidi.MidiMessage().allNotesOff(2)),
	('isAllSoundOff', rtmidi.MidiMessage().allSoundOff(1)),
	('isAllSoundOff', rtmidi.MidiMessage().allSoundOff(2)),
	('isChannelPressure', rtmidi.MidiMessage().channelPressureChange(1, 50)),
	('isChannelPressure', rtmidi.MidiMessage().channelPressureChange(2, 100)),
	('isController', rtmidi.MidiMessage().controllerEvent(1, 10, 100)),
	('isController', rtmidi.MidiMessage().controllerEvent(2, 11, 127)),
#	('isSysEx', rtmidi.MidiMessage().createSysExMessage( ??? )),
	(None, rtmidi.MidiMessage().endOfTrack()),
	(None, rtmidi.MidiMessage().masterVolume(.5)),
	(None, rtmidi.MidiMessage().masterVolume(.7)),
	(None, rtmidi.MidiMessage().midiChannelMetaEvent(1)),
	(None, rtmidi.MidiMessage().midiChannelMetaEvent(2)),
	(None, rtmidi.MidiMessage().midiClock()),
	(None, rtmidi.MidiMessage().midiContinue()),
#	(None, rtmidi.MidiMessage().midiMachineControlCommand( ???)),
	(None, rtmidi.MidiMessage().midiMachineControlGoto(1, 10, 3, 10)),
	(None, rtmidi.MidiMessage().midiStart()),
	(None, rtmidi.MidiMessage().midiStop()),
	('isNoteOn', m.noteOn(1, 10, 100)),
	('isNoteOn', rtmidi.MidiMessage().noteOn(2, 60, 127)),
	('isNoteOff', rtmidi.MidiMessage().noteOff(1, 10)),
	('isNoteOff', rtmidi.MidiMessage().noteOff(2, 60)),
	('isProgramChange', rtmidi.MidiMessage().programChange(1, 10)),
	('isProgramChange', rtmidi.MidiMessage().programChange(2, 20)),
	('isPitchWheel', rtmidi.MidiMessage().pitchWheel(1, 10)),
	('isPitchWheel', rtmidi.MidiMessage().pitchWheel(2, 20)),
	(None, rtmidi.MidiMessage().quarterFrame(1, 10)),
	(None, rtmidi.MidiMessage().quarterFrame(2, 20)),
	(None, rtmidi.MidiMessage().songPositionPointer(10)),
	(None, rtmidi.MidiMessage().songPositionPointer(20)),
	(None, rtmidi.MidiMessage().tempoMetaEvent(10)),
	(None, rtmidi.MidiMessage().tempoMetaEvent(20)),
	(None, rtmidi.MidiMessage().timeSignatureMetaEvent(10, 19)),
	(None, rtmidi.MidiMessage().timeSignatureMetaEvent(11, 15)),

	)


print('First line: transmitted message, second line: received message.\nIf both lines are identical, the transmission has been successful.')
for checkfunction, message in messages:
	print()
#	print(message.__name__)
	if checkfunction:
		exec('print("Correct message type:", message.%s())' % (checkfunction)) 
	print(message, '(transmitted)')
	midiout.sendMessage(message)
	time.sleep(0.1)

midiout.closePort()
receiver.stopReceiving()
