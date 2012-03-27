#!/usr/bin/python

import pynvc

def listen():
  while True:
    received_data = pynvc.receive(5000)
    if received_data != None:
      print "Received ", len(received_data), " bytes: ", received_data

pynvc.init()
listen()