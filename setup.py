#!/bin/env python
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


from distutils.core import setup, Extension

midi = Extension(
    'rtmidi',
    sources=[
        'MidiMessage.cpp',
        'PyMidiMessage.cpp',
        'rtmidimodule.cpp'
    ],
    library_dirs=['/usr/local/lib'],
    libraries=['rtmidi'],
    include_dirs=['/usr/local/include'],
)

setup(
    name='rtmidi',
    version='0.2.1',
    description='Python RtMidi interface',
    ext_modules=[midi]
)
