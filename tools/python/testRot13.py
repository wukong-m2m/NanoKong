#!/usr/bin/python

import sys
import pynvc



def testRot13(destination, input):
  print "--- Input: ", input
  pynvc.sendcmd(destination, pynvc.APPMSG, [ord(c) for c in input])
  print "--- Sent"
  reply = pynvc.receive(5000) # Receive reply
  print "--- Received reply"
  pynvc.sendcmd(destination, pynvc.APPMSG_R, [pynvc.APPMSG_STATUS_ACK])
  print "Reply:", [chr(c) for c in reply[1:]] # Strip of the leading 0x7e (APPMSG) command

pynvc.init()
testRot13(int(sys.argv[1]), sys.argv[2] if len(sys.argv)>2 else "Hallo")
