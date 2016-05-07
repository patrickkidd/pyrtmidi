import os, sys, imp
BUILD_PATHS = [os.path.join(os.getcwd(), 'build/lib.linux-x86_64-2.4/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.5.0-Power_Macintosh-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.darwin-8.8.1-i386-2.3/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.6-universal-2.6/'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.9-x86_64-3.4'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.10-intel-2.6'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.10-intel-2.7'),
               os.path.join(os.getcwd(), 'build/lib.macosx-10.11-x86_64-3.5')
               ]
sys.path = BUILD_PATHS + sys.path
bleh = imp.find_module('rtmidi')
print('%s: Using rtmidi module at:' % __name__, bleh[1])
import rtmidi
