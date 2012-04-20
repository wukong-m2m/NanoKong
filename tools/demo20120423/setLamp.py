#!/usr/bin/python

import sys
import pynvc

def setLamp(destination, input):
  pynvc.sendcmd(destination, pynvc.APPMSG, [1, input])

if __name__ == "__main__":
  pynvc.init()
  if len(sys.argv)<3 or not sys.argv[2] in ["0", "1"]:
    print "setLamp <nodeid> [0|1]"
  else:
    setLamp(int(sys.argv[1]), int(sys.argv[2]))
