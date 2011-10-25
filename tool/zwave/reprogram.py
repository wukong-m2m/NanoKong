#!/usr/bin/python

import os
import time
import sys

import pyzwave

FOPEN        = 0x70
FCLOSE       = 0x71
FSEEK        = 0x72
RDFILE       = 0x73
WRFILE       = 0x74
GETRUNLVL    = 0x75
SETRUNLVL    = 0x76

RUNLVL_VM    = 0x02
RUNLVL_CONF  = 0x03
RUNLVL_RESET = 0x04

def sendcmd(cmd):
  return pyzwave.senddata(1, [0x88] + cmd)

def getRunLevelTest():
  sendcmd([GETRUNLVL])
  received_data = pyzwave.receive(5000)
  if not received_data == None:
    print "Received runlevel:", received_data[0]

def reprogramNvmdefault():
  sendcmd([SETRUNLVL, RUNLVL_CONF])
  sendcmd([FOPEN, 0])
  sendcmd([FSEEK, 0, 0])
  lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(sys.argv[1]).readlines() if l.startswith('0x')]
  for l in lines:
    sendcmd([WRFILE] + l)
  sendcmd([FCLOSE])
  sendcmd([SETRUNLVL, RUNLVL_RESET])

pyzwave.init("192.168.0.231")
#reprogramNvmdefault()
getRunLevelTest()
