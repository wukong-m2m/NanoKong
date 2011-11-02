#!/usr/bin/python

import pynvc3

def listen():
  while True:
    received_data = pynvc3.receive(5000)
    if received_data != None:
      print "Received ", len(received_data), " bytes: ", received_data

pynvc3.init()
listen()