#!/usr/bin/python

import sys
import pynvc

def getRunLevelTest(destination):
  pynvc.sendcmd(destination, pynvc.GETRUNLVL)
  received_data = pynvc.receive(5000)
  if not received_data == None:
    print "Received runlevel:", received_data[1]

pynvc.init()
getRunLevelTest(int(sys.argv[1]))
