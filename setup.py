# -*- coding: utf-8 -*-		
#!/bin/env python
#   Copyright (C) 2018 by Patrick Stinson                                  
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



import os, sys
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
include_dirs = []

def checkPathsAndGetDirectories(paths):
    dirs = []
    missingPaths = []
    for path in paths : 
        directory , filename = os.path.split(path) 
        if os.path.exists(path):
            dirs.append(directory)
        else : 
            missingPaths.append(path)
    if missingPaths : 
        print('ATTENTION : Unable to localise :\n%s\ndid you install visual Studio 2017 with the "Python development workload and the Native development tools" option cheked ?'%('\n'.join(missingPaths)))
    #print('\n'.join(dirs))
    return(dirs)

if OSNAME == 'Linux':
    define_macros = [('__LINUX_ALSA__', '')]
    libraries = ['asound', 'pthread']
elif OSNAME == 'Darwin':
    define_macros = [('__MACOSX_CORE__', '')]
    libraries = ['pthread']
    extra_compile_args = ['-Wno-missing-braces']
    extra_link_args = ['-framework', 'CoreAudio',
                       '-framework', 'CoreMidi',
                       '-framework', 'CoreFoundation']
elif OSNAME == 'Windows':
    architecture = str(sys.maxsize.bit_length() + 1) + "-bit"  # retrieve sur python architecture (32-bit or 64-bit)
    define_macros = [('__WINDOWS_MM__', ''),
                     ('PK_WINDOWS', '1')]
    libraries = ['winmm', "python" + str(sys.version_info[0]) + str(sys.version_info[1])] # add the good pythonXX
    extra_compile_args = ['/EHsc']
    if sys.version_info >= (3, 5): # Use Visual Studio 14.0

        windowsKitsFolder = "C:/Program Files (x86)/Windows Kits/10/Include/"
        if os.path.exists(windowsKitsFolder):
            windowsKitVersions = sorted(os.listdir(windowsKitsFolder))
            if windowsKitVersions: 
                lastWindowsKitVersion = windowsKitVersions[-1] # retrieve last Windows Kit version
 
                includes = [
                        'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/include/exception',          # for "exception" file
                        'C:/Program Files (x86)/Windows Kits/10/Include/%s/ucrt/corecrt.h'%lastWindowsKitVersion,  # for corecrt.h
                        'C:/Program Files (x86)/Windows Kits/10/Include/%s/um/windows.h'%lastWindowsKitVersion,    # for windows.h
                        'C:/Program Files (x86)/Windows Kits/10/Include/%s/shared/winapifamily.h'%lastWindowsKitVersion # for winapifamily.h
                        ]
                include_dirs = checkPathsAndGetDirectories(includes)
                if architecture == "32-bit":
                    librariesPaths = [    
                        'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib/msvcprt.lib',        # for  in 32 bit 
                        'C:/Program Files (x86)/Windows Kits/10/Lib/%s/um/x86/winmm.lib'%lastWindowsKitVersion,    # for  in 32 bit 
                        'C:/Program Files (x86)/Windows Kits/10/Lib/%s/ucrt/x86/ucrt.lib'%lastWindowsKitVersion,  # for   in 32 bit 
                        ]
                    library_dirs = checkPathsAndGetDirectories(librariesPaths)
                    
                elif architecture == "64-bit":
                    librariesPaths = [    
                        'C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/lib/amd64/msvcprt.lib',  # for  in 64 bit                    
                        'C:/Program Files (x86)/Windows Kits/10/Lib/%s/um/x64/winmm.lib'%lastWindowsKitVersion,    # for  in 64 bit 
                        'C:/Program Files (x86)/Windows Kits/10/Lib/%s/ucrt/x64/ucrt.lib'%lastWindowsKitVersion,  # for   in 64	bit 
                        ]
                    library_dirs = checkPathsAndGetDirectories(librariesPaths)
            else : 
                print('ATTENTION the %s folder is empty did you\n%s\ndid you install visual Studio 2017 with the "Python development workload and the Native development tools" option cheked ?'%windowsKitsFolder)

        else : 
              print('ATTENTION the %s folder do not exists\n%s\ndid you install visual Studio 2017 with the "Python development workload and the Native development tools" option cheked ?'%windowsKitsFolder)

    elif sys.version_info >= (3, 3):  # Use Visual Studio 10.0
        pass
    else :   # Use visual Studio 9.0
        library_dirs = [    
                'C:/Users/%s/AppData/Local/Programs/Common/Microsoft/Visual C++ for Python/9.0/VC/lib/amd64'% os.environ.get('USERNAME').lower(),  # for msvcprt.lib in 64 bit  
                ]		
        print(library_dirs)
elif OSNAME == 'Irix':
    define_macros = [('__IRIX_MD__', '')]
    libraries = ['pthread', 'md']

if '--jack-midi' in sys.argv:
    define_macros.append(('__UNIX_JACK__', ''))
    libraries.append('jack')
    sys.argv.pop(sys.argv.index('--jack-midi'))

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
                 include_dirs = include_dirs,
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
      download_url='https://github.com/patrickkidd/pyrtmidi/tarball/2.3.3',
      keywords=['midi audio hardware'],
      version='2.3.3',
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
