#!/usr/bin/python

import sys
import pynvc

def testProfile(destination, input):
  pynvc.sendcmd(destination, pynvc.WKPF_GET_PROPERTY, [00, 42, 1, 2]);
  print "--- Sent"
  reply = pynvc.receive(5000) # Receive reply: should be a WKPF_GET_PROPERTY_R
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 

pynvc.init()
testProfile(int(sys.argv[1]))