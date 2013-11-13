import rtmidi

m = rtmidi.MidiMessage.programChange(7, 1)
print(m)
