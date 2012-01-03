#!/usr/bin/python

import sys
import pynvc3

def setPeopleInRoom(destination, input):
  pynvc3.sendcmd(destination, pynvc3.APPMSG, [5, input])

if __name__ == "__main__":
  pynvc3.init()
  if len(sys.argv)<3 or not sys.argv[2] in ["0", "1"]:
    print "setPeopleInRoom <nodeid> [0|1]"
  else:
    setPeopleInRoom(int(sys.argv[1]), int(sys.argv[2]))