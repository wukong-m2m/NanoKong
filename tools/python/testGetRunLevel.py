#!/usr/bin/python

import sys
import pynvc

def getRunLevelTest(destination):
  pynvc.sendcmd(destination, pynvc.GETRUNLVL)
  src, received_data = pynvc.receive(5000)
  if not received_data == None:
    print "Received runlevel:", received_data[1]

pynvc.init(0)
getRunLevelTest(int(sys.argv[1]))
