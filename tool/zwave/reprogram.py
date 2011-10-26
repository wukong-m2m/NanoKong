#!/usr/bin/python

import os
import time
import sys

import pyzwave

FOPEN        = 0x70
FCLOSE       = 0x72
FSEEK        = 0x74
RDFILE       = 0x76
RDFILE_R     = 0x77
WRFILE       = 0x78
GETRUNLVL    = 0x7a
GETRUNLVL_R  = 0x7b
SETRUNLVL    = 0x7c
SETRUNLVL_R  = 0x7d
APPMSG       = 0x7e

RUNLVL_VM    = 0x02
RUNLVL_CONF  = 0x03
RUNLVL_RESET = 0x04

def sendcmd(cmd, payload=[]):
  return pyzwave.senddata(1, [0x88, cmd] + payload)

def listen():
  while True:
    received_data = pyzwave.receive(5000)
    if received_data != None:
      print "Received ", len(received_data), " bytes: ", received_data

def getRunLevelTest():
  sendcmd(GETRUNLVL)
  listen()
  received_data = pyzwave.receive(5000)
  if not received_data == None:
    print "Received runlevel:", received_data[0]

def reprogramNvmdefault():
  sendcmd(SETRUNLVL, [RUNLVL_CONF])
  sendcmd(FOPEN, [0])
  sendcmd(FSEEK, [0, 0])
  lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(sys.argv[1]).readlines() if l.startswith('0x')]
  for l in lines:
    sendcmd(WRFILE, l)
  sendcmd(FCLOSE)
  sendcmd(SETRUNLVL, [RUNLVL_RESET])

pyzwave.init("192.168.0.231")
#reprogramNvmdefault()
getRunLevelTest()
#listen()
