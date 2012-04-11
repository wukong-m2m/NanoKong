#!/usr/bin/python

import os
import platform

CWD = os.getcwd()


print "== NanoKong Bootstrap Script =="

print "This script assumes you are going to compile avr_mega2560, if you need to compile for a different arch, change the VERSION in this script"

VERSION = '2560'

if platform.system() == 'Linux':
  print "== Determing the port =="
  port = ''
elif platform.system() == 'Window':
  print "== Determing the port =="
  port = ''
elif platform.system() == 'Darwin':
  print "== Determing the port =="
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

  print "== Setting up Arduino =="

  # Default path
  arduino = '/Applications/Arduino.app'

  if not os.path.exists(arduino):
    print "Arduino not found in /Applications folder, please provide the path to Arduino.app:"
    arduino = os.path.join(raw_input('--> '), 'Arduino.app')
  arduino = os.path.join(arduino, 'Contents', 'Resources', 'Java', 'hardware', 'tools', 'avr', 'bin')
  print "arduino is %s" % (arduino)

print "== Building NanoVMTool =="
nanovmtool = os.path.join(CWD, 'nanovmtool')
os.system("cd %s" % (nanovmtool))
os.system("ant")
os.system("cd ..")

print "== Compiling VM =="
vm = os.path.join(CWD, 'vm', 'build', 'avr_mega%s' % (VERSION))
os.chdir(vm)
os.system("make PREFIX=%s PORT=%s" % (arduino, port))

print "== Uploading VM =="
vm = os.path.join(CWD, 'vm', 'build', 'avr_mega%s' % (VERSION))
print vm
print arduino, port
os.chdir(vm)
os.system("make avrdude PREFIX=%s PORT=%s" % (arduino, port))

os.chdir(CWD)

choice = raw_input("Do you want to see the program's ouput? [Y/N]")

if choice == 'Y' or choice == 'y':
  os.system("screen %s 115200" % (port))
