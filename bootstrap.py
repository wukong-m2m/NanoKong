#!/usr/bin/python

import os
import platform

CWD = os.getcwd()


print "== NanoKong Bootstrap Script =="

print "\n\n\nThis script assumes you are going to compile avr_mega2560, if you need to compile for a different arch, change the VERSION in this script\n\n\n"

VERSION = '2560'

if platform.system() == 'Linux':
  print "== Determing device port =="
  port = ''
elif platform.system() == 'Window':
  print "== Determing device port =="
  port = ''
elif platform.system() == 'Darwin':
  print "== Determing device port =="
  paths = []
  index = 0

  for path in os.listdir('/dev'):
    if path.find('tty.usbmodem') == 0:
      paths.append(path)

  if len(paths) > 1:
    print "There are more than one device found with similar hardware, please select one from the list below:"
    for idx, opt in enumerate(paths):
      print str(idx)+'.', opt
    choice = raw_input("Please enter the index [0] --> ")
    index = int(choice) if choice != '' else 0
  elif len(paths) == 1:
    index = 0
  else:
    print "no device is found, please plug the device into the USB slot before running this script."
    exit(1)

  port = os.path.join('/dev', paths[index])
  print "port is %s" % (port)

  print "\n\n\n"

  print "== Setting up Arduino =="

  # Default path
  arduino = '/Applications/Arduino.app'

  if not os.path.exists(arduino):
    print "Arduino not found in /Applications folder, please provide the path to Arduino.app:"
    arduino = os.path.join(raw_input('--> '), 'Arduino.app')
  arduino = os.path.join(arduino, 'Contents', 'Resources', 'Java', 'hardware', 'tools', 'avr', 'bin')
  print "arduino is %s" % (arduino)

print "\n\n\n"

print "== Building NanoVMTool =="
nanovmtool = os.path.join(CWD, 'nanovmtool')
os.chdir(nanovmtool)
os.system("ant")

print "\n\n\n"

print "== Compiling VM =="
vm = os.path.join(CWD, 'vm', 'build', 'avr_mega%s' % (VERSION))
os.chdir(vm)
os.system("make PREFIX=%s PORT=%s" % (arduino, port))

print "\n\n\n"

print "== Uploading VM =="
vm = os.path.join(CWD, 'vm', 'build', 'avr_mega%s' % (VERSION))
print vm
print arduino, port
os.chdir(vm)
os.system("make avrdude PREFIX=%s PORT=%s" % (arduino, port))

os.chdir(CWD)

print "\n\n\n"

choice = raw_input("Do you want to see the program's ouput? [Y/N]")

if choice == 'Y' or choice == 'y':
  os.system("screen %s 115200" % (port))
