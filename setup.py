#!/bin/env python
#   Copyright (C) 2023 by Patrick Stinson
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
from setuptools import setup, Extension


if hasattr(os, "uname"):
    OSNAME = os.uname()[0]
else:
    OSNAME = "Windows"


define_macros = []
libraries = []
extra_link_args = []
extra_compile_args = []
library_dirs = []


if OSNAME == "Linux":
    define_macros = [("__LINUX_ALSA__", "")]
    libraries = ["asound", "pthread"]
elif OSNAME == "Darwin":
    define_macros = [("__MACOSX_CORE__", "")]
    libraries = ["pthread"]
    extra_compile_args = ["-Wno-missing-braces", "-Wdeprecated-declarations"]
    extra_link_args = [
        "-framework",
        "CoreAudio",
        "-framework",
        "CoreMidi",
        "-framework",
        "CoreFoundation",
    ]
elif OSNAME == "Windows":
    define_macros = [("__WINDOWS_MM__", ""), ("PK_WINDOWS", "1")]
    library_dirs = []
    libraries = ["winmm"]
    extra_compile_args = ["/EHsc"]
elif OSNAME == "Irix":
    define_macros = [("__IRIX_MD__", "")]
    libraries = ["pthread", "md"]

if "--jack-midi" in sys.argv:
    define_macros.append(("__UNIX_JACK__", ""))
    libraries.append("jack")
    sys.argv.pop(sys.argv.index("--jack-midi"))


setup(
    name="rtmidi",
    author="Patrick Stinson",
    author_email="patrickkidd@gmail.com",
    url="https://github.com/patrickkidd/pyrtmidi",
    # download_url="https://github.com/patrickkidd/pyrtmidi/tarball/2.3.4",
    keywords=["midi audio hardware"],
    version="2.5.0",
    description="Realtime MIDI I/O for Python on Windows, OS X, and Linux",
    description_content_type="text/plain",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    ext_modules=[
        Extension(
            name="rtmidi._rtmidi",
            sources=[
                os.path.join("src", fname)
                for fname in [
                    "RtMidi.cpp",
                    "MidiMessage.cpp",
                    "PyMidiMessage.cpp",
                    "rtmidimodule.cpp",
                ]
            ],
            headers=[
                os.path.join("src", fname)
                for fname in [
                    "RtMidi.h",
                    "pkglobals.h",
                    "MidiMessage.h",
                    "PyMidiMessage.h",
                ]
            ],
            library_dirs=library_dirs,
            libraries=libraries,
            define_macros=define_macros,
            extra_link_args=extra_link_args,
            extra_compile_args=extra_compile_args,
        )
    ],
    packages=["rtmidi"],
    scripts=["pkechomidi.py"],
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.11",
    ],
)


DESCRIPTION = """
Python RtMidi interface
```
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
```
"""
