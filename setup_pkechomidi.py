import os
import sys
from cx_Freeze import setup, Executable


if hasattr(os, 'uname'):
    OSNAME = os.uname()[0]
else:
    OSNAME = 'Windows'

# Dependencies are automatically detected, but it might need fine tuning.
build_exe_options = {
    "packages": ["os"],
    "excludes": ["tkinter"],
    "include_files": []
}

if OSNAME == 'Windows':
    build_exe_options['include_files'] = [
        os.path.relpath('C:\Python34\Lib\site-packages\PyQt5\LibEGL.dll')
    ]

base = None
# Uncomment for a GUI-only (no console) app
#if sys.platform == "win32":
#    base = "Win32GUI"

def get_data_files():
    return [('', [''])]

setup(  name = "PKMidiStroke",
        version = "0.1",
        description = "Trigger the triggers!",
        options = {"build_exe": build_exe_options},
        executables = [Executable("pkechomidi", base=base)],
        data_files=get_data_files())
