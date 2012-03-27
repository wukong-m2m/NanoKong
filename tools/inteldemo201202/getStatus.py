#!/usr/bin/python

import sys
import pynvc

def getStatus(destination):
  pynvc.receive(10)
  pynvc.sendcmd(destination, pynvc.APPMSG, [3]);
  reply = pynvc.receive(100) # Receive reply
  if reply != None and reply[1] == 4:
    return reply[2:]
  else:
    reply = pynvc.receive(100) # Receive reply
    if reply != None and reply[1] == 4:
      return reply[2:]
    else:
      return None

if __name__ == "__main__":
  pynvc.init()
  reply = getStatus(int(sys.argv[1]))
  if reply != None:
    print "Threshold: ", reply[0]
    print "Light sensor: ", reply[1]
    print "Lamp: On" if reply[2] == 1 else "Lamp: Off"
    if len(reply) > 3:
      print "Room occupied: Yes" if reply[3] == 1 else "Room occupied: No"
  else:
    print "Received incorrect reply: ", reply
