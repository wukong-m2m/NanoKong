#!/usr/bin/python

import sys
import pynvc

def setPeopleInRoom(destination, input):
  pynvc.sendWithRetryAndCheckedReceiveAPPMSG(
                    destination=destination,
                    command=5,
                    allowedReplies=[15],
                    payload=[input])

if __name__ == "__main__":
  pynvc.init()
  if len(sys.argv)<3 or not sys.argv[2] in ["0", "1"]:
    print "setPeopleInRoom <nodeid> [0|1]"
  else:
    setPeopleInRoom(int(sys.argv[1]), int(sys.argv[2]))