#!/usr/bin/python

import pynvc3

def getRunLevelTest():
  pynvc3.sendcmd(pynvc3.GETRUNLVL)
  received_data = pynvc3.receive(5000)
  if not received_data == None:
    print "Received runlevel:", received_data[1]

pynvc3.init()
getRunLevelTest()
