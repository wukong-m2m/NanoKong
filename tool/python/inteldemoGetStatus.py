#!/usr/bin/python

import sys
import pynvc3

def getStatus(destination):
  pynvc3.sendcmd(destination, pynvc3.APPMSG, [3]);
  reply = pynvc3.receive(1000) # Receive reply
  if reply != None and reply[1] == 4:
    print "Threshold: ", reply[2]
    print "Light sensor: ", reply[3]
    print "Lamp: On" if reply[4] == 1 else "Lamp: Off"
    if len(reply) > 5:
      print "Room occupied: Yes" if reply[5] == 1 else "Room occupied: No"
  else:
    print "Received incorrect reply: ", reply

pynvc3.init()
getStatus(int(sys.argv[1]))
