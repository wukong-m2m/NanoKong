#!/usr/bin/python

import sys
import pynvc3

def testLamp(destination, threshold):
  if threshold > 255:
    print "sendThresholdCommand <nodeid> <threshold>"
    print "0<=threshold<=255"
  pynvc3.sendcmd(destination, pynvc3.APPMSG, [2, threshold])

pynvc3.init()
testLamp(int(sys.argv[1]), int(sys.argv[2]) if len(sys.argv)>2 else 0)