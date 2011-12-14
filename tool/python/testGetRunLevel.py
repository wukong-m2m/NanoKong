#!/usr/bin/python

import sys
import pynvc3

def getRunLevelTest(destination):
  pynvc3.sendcmd(destination, pynvc3.GETRUNLVL)
  received_data = pynvc3.receive(5000)
  if not received_data == None:
    print "Received runlevel:", received_data[1]

pynvc3.init()
getRunLevelTest(int(sys.argv[1]))
