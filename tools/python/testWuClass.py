#!/usr/bin/python

import sys
import pynvc

def testWuClass(destination):

  pynvc.sendcmd(destination, pynvc.WKPF_GET_WUCLASS_LIST, [0x05, 0x05]);
  print "--- Sent get"
  src, reply = pynvc.checkedReceive([pynvc.WKPF_GET_WUCLASS_LIST_R], 5000) # Receive reply: should be a WKPF_GET_WUCLASS_LIST_R
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 

	#wuclass:00~02,property:00~01
	#seq=05 05, wuclassID=00 02, roleID=a1, propertyID=01, value=aabbccdd
  pynvc.sendcmd(destination, pynvc.WKPF_WRITE_PROPERTY, [0x05, 0x05, 0x00, 0x02, 0xa1, 0x01, 0xaa, 0xbb, 0xcc, 0xdd]);
  print "--- Sent write"
  src, reply = pynvc.checkedReceive([pynvc.WKPF_WRITE_PROPERTY_R], 5000) # Receive reply: should be a WKPF_WRITE_PROPERTY
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 


  pynvc.sendcmd(destination, pynvc.WKPF_READ_PROPERTY, [0x05, 0x05, 0x00, 0x02, 0xa1, 0x00]);
  print "--- Sent read"
  src, reply = pynvc.checkedReceive([pynvc.WKPF_READ_PROPERTY_R], 5000) # Receive reply: should be a WKPF_READ_PROPERTY_R
  print "--- Received reply"
  print "Reply:", [chr(c) for c in reply[1:]] 



pynvc.init(0)
testWuClass(int(sys.argv[1]))
