#!/usr/bin/python

import pynvc3

def testRot13():
  input = "Hallo";
  print "Input: ", input
  pynvc3.sendcmd(pynvc3.APPMSG, [ord(c) for c in input])
  pynvc3.receive(5000) # Receive ack of APPMSG
  reply = pynvc3.receive(5000) # Receive reply
  pynvc3.sendcmd(pynvc3.APPMSG, [pynvc3.APPMSG_STATUS_ACK])
  print "Reply:", [chr(c) for c in reply[1:]] # Strip of the leading 0x7e (APPMSG) command

pynvc3.init()
testRot13()