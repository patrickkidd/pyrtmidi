#!/bin/env python
#   Copyright (C) 2017 by Patrick Stinson                                 
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



import os
from distutils.core import setup, Extension

if hasattr(os, 'uname'):
    OSNAME = os.uname()[0]
else:
    OSNAME = 'Windows'


define_macros = []
libraries = []
extra_link_args = []
extra_compile_args = []
library_dirs = []


if OSNAME == 'Linux':
    define_macros = [("__LINUX_ALSASEQ__", '')]
    libraries = ['asound', 'pthread']
elif OSNAME == 'Darwin':
    define_macros = [('__MACOSX_CORE__', '')]
    libraries = ['pthread']
    extra_compile_args = ['-Wno-missing-braces']
    extra_link_args = ['-framework', 'CoreAudio',
                       '-framework', 'CoreMidi',
                       '-framework', 'CoreFoundation']
elif OSNAME == 'Windows':
    define_macros = [('__WINDOWS_MM__', ''),
                     ('PK_WINDOWS', '1')]
    library_dirs = ['C:\Program Files\Microsoft SDKs\Windows\v7.1\Lib']
    libraries = ['winmm', 'python34']
    extra_compile_args = ['/EHsc']
elif OSNAME == 'Irix':
    define_macros = [('__IRIX_MD__', '')]
    libraries = ['pthread', 'md']

CPP_SRC_DIR = 'cpp_src'
CPP_FNAMES = ['RtMidi.cpp',
              'MidiMessage.cpp',
              'PyMidiMessage.cpp',
              'rtmidimodule.cpp',
              ]

CPP_HEADERS = ['RtMidi.h',
               'pkglobals.h',
               'MidiMessage.h',
               'PyMidiMessage.h'
               ]

cpp_sources = [os.path.join(CPP_SRC_DIR, fname) for fname in CPP_FNAMES]
cpp_headers = [os.path.join(CPP_SRC_DIR, fname) for fname in CPP_HEADERS]

midi = Extension(name='rtmidi._rtmidi',
                 sources=cpp_sources,
                 headers=cpp_headers,
                 library_dirs=library_dirs,
                 libraries=libraries,
                 define_macros=define_macros,
                 extra_link_args=extra_link_args,
                 extra_compile_args=extra_compile_args,
                 )

setup(name='rtmidi',
      author='Patrick Stinson',
      author_email='patrickkidd@gmail.com',
      url='https://github.com/patrickkidd/pyrtmidi',
      keywords=['midi audio hardware'],
      version='2.3.1',
      description = """Python RtMidi interface
def print_message(midi):
    if midi.isNoteOn():
        print 'ON: ', midi.getMidiNoteName(midi.getNoteNumber()), midi.getVelocity()
    elif midi.isNoteOff():
        print 'OFF:', midi.getMidiNoteName(midi.getNoteNumber())
    elif midi.isController():
        print 'CONTROLLER', midi.getControllerNumber(), midi.getControllerValue()


import rtmidi
midiin = rtmidi.RtMidiIn()

ports = range(midiin.getPortCount())
if ports:
    for i in ports:
        print midiin.getPortName(i)
    midiin.openPort(1)
    while True:
        m = midiin.getMessage(250) # some timeout in ms
        if m != None:
            print_message(m)
else:
    print 'NO MIDI INPUT PORTS!'
""",
      ext_modules=[midi],
      packages=['rtmidi'],
      scripts=['pkechomidi.py'],
      classifiers=['Development Status :: 5 - Production/Stable',
                   'Intended Audience :: Developers',
                   'Programming Language :: Python :: 3',
                   'Programming Language :: Python :: 3.4',
                   ]
      )
