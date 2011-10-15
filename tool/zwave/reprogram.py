#!/usr/bin/python

import os
import time
import sys

FOPEN        = " 70"
FCLOSE       = " 71"
FSEEK        = " 72"
RDFILE       = " 73"
WRFILE       = " 74"
GETRUNLVL    = " 75"
SETRUNLVL    = " 76"

RUNLVL_VM    = " 02"
RUNLVL_CONF  = " 03"
RUNLVL_RESET = " 04"

def sendcmd(cmd):
  cmd = "./testrtt host 192.168.0.231 nowait raw 01 88 " + cmd
  result = ""
  print cmd
  while not "Transmit complete ok." in result:
    print "============ " + cmd
    fin,fout = os.popen4(cmd)
    result = fout.read()
    time.sleep(1) # one step at a time...

def getRunLevelTest():
  sendcmd(GETRUNLVL)

def reprogramNvmdefault():
  sendcmd(SETRUNLVL + RUNLVL_CONF)
  sendcmd(FOPEN + " 00")
  sendcmd(FSEEK + " 00 00")
  lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(sys.argv[1]).readlines() if l.startswith('0x')]
  for l in lines:
    sendcmd(WRFILE + l)
  sendcmd(FCLOSE)
  sendcmd(SETRUNLVL + RUNLVL_RESET)

reprogramNvmdefault()
#getRunLevelTest()

