#!/usr/bin/python

import sys
import pynvc3

def testLamp(destination, input):
  if input == "On":
    pynvc3.sendcmd(destination, pynvc3.APPMSG, [1, 1])
    pynvc3.receive(5000) # Receive ack of APPMSG
  elif input == "Off":
    pynvc3.sendcmd(destination, pynvc3.APPMSG, [1, 0])
    pynvc3.receive(5000) # Receive ack of APPMSG
  else:
    print "inteldemoSendLampCommand <nodeid> [On|Off]"

pynvc3.init()
testLamp(int(sys.argv[1]), sys.argv[2] if len(sys.argv)>2 else "Off")