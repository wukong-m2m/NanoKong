#!/usr/bin/python

import sys
import pynvc

def setThreshold(destination, threshold):
  pynvc.sendWithRetryAndCheckedReceiveAPPMSG(
                    destination=destination,
                    command=2,
                    allowedReplies=[12],
                    payload=[threshold])

if __name__ == "__main__":
  pynvc.init()
  if len(sys.argv)<3 or int(sys.argv[2]) < 0 or int(sys.argv[2]) > 255:
    print "setThreshold <nodeid> <threshold>"
    print "0<=threshold<=255"
  else:
    setThreshold(int(sys.argv[1]), int(sys.argv[2]))
