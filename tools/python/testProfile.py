#!/usr/bin/python

import sys
import pynvc

def testProfile(destination):

  pynvc.sendcmd(destination, pynvc.WKPF_WRITE_PROPERTY, [0x00, 0x42, 0x01, 0x02, 0xaa, 0xbb, 0xcc, 0xdd]);
  print "--- Sent write"
  reply = pynvc.receive(5000) # Receive reply: should be a WKPF_GET_PROPERTY_R
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 


  pynvc.sendcmd(destination, pynvc.WKPF_READ_PROPERTY, [0x00, 0x42, 0x01, 0x02]);
  print "--- Sent read"
  reply = pynvc.receive(5000) # Receive reply: should be a WKPF_GET_PROPERTY_R
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 

  pynvc.sendcmd(destination, pynvc.WKPF_GET_PROFILE_LIST, [0x00, 0x42, 0x01, 0x02]);
  print "--- Sent get"
  reply = pynvc.receive(5000) # Receive reply: should be a WKPF_GET_PROPERTY_R
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 

pynvc.init()
testProfile(int(sys.argv[1]))
