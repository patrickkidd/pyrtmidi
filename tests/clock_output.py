#!/usr/bin/env pkpython
#   Copyright (C) 2011 by Patrick Stinson                                 
#   patrickkidd@gmail.com                                                   
#                                                                         
#   This program is free software; you can redistribute it and/or modify  
#   it under the terms of the GNU General Public License as published by  
#   the Free Software Foundation; either version 2 of the License, or     
#   (at your option) any later version.                                   
#                                                                         
#   This program is distributed in the hope that it will be useful,       
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
#   GNU General Public License for more details.                          
#                                                                         
#   You should have received a copy of the GNU General Public License     
#   along with this program; if not, write to the                         
#   Free Software Foundation, Inc.,                                       
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  

import sys
import rtmidi
import threading
from PyQt4.QtGui import *
from PyQt4.QtCore import *


MIDI_CLOCK = 0xF8
MIDI_START = 0xFA
MIDI_STOP = 0xFC


class MidiClock(threading.Thread):
    
    tempo = 140.0
    dev = None
    
    def __init__(self):
        threading.Thread.__init__(self)
        self._running = True
        self.start()

    def __del__(self):
        self.quit()

    def run(self):
        while self._running:
            spb = 60.0 / self.tempo
            spc = spb / 24.0
            usec = spc * 1000000
            self.send(MIDI_CLOCK)
            time.sleep(spc)
            #self.usleep(long(usec))

    def quit(self):
        self._running = False

    def startClock(self):
        self.send(MIDI_START)

    def stopClock(self):
        self.send(MIDI_STOP)

    def send(self, *args):
        pass


class RtMidiClock(MidiClock):
    def __init__(self):
        self.lock = threading.Lock()
        self.setDevice(None)
        MidiClock.__init__(self)

    def setDevice(self, i=None):
        self.lock.acquire()
        self.dev = rtmidi.RtMidiOut()
        if isinstance(i, int):
            self.dev.openPort(i)
        elif isinstance(i, str):
            self.dev.openVirtualPort(i)
        else:
            self.dev.openVirtualPort('pk.RtMidiOut')
        self.lock.release()

    def send(self, *args):
        self.dev.sendMessage(*args)


class ClockWidget(QWidget):

    def __init__(self, clock, parent=None):
        QWidget.__init__(self, parent)

        self.label = QLabel(self)
        self.slider = QSlider(Qt.Horizontal, self)
        self.startButton = QPushButton('start', self)
        self.stopButton = QPushButton('stop', self)

        self.clock = clock

        QObject.connect(self.slider, SIGNAL('valueChanged(int)'),
                        self.setTempo)
        QObject.connect(self.startButton, SIGNAL('clicked()'),
                        self.clock.startClock)
        QObject.connect(self.stopButton, SIGNAL('clicked()'),
                        self.clock.stopClock)

        self.slider.setRange(4000, 21000)
        self.slider.setValue(14000)

        Layout = QVBoxLayout(self)
        Layout.addWidget(self.label)
        Layout.addWidget(self.slider)

        ButtonLayout = QHBoxLayout()
        ButtonLayout.addWidget(self.startButton)
        ButtonLayout.addWidget(self.stopButton)
        Layout.addLayout(ButtonLayout)

    def setTempo(self, x):
        """ """
        tempo = x / 100.0
        self.clock.tempo = tempo
        self.label.setText(str(tempo))



class ClockConfig(QWidget):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)

        self.portBox = QComboBox(self)


if __name__ == '__main__':
    import time
    app = QApplication(sys.argv)
    clock = RtMidiClock()
    w = ClockWidget(clock)
    w.show()
    app.exec_()
    clock.quit()
