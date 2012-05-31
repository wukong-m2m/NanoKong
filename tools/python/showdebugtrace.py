#!/usr/bin/python

import pynvc

messages_in_progress = {}

pynvc.init(0)
while(true):
  src, message = pynvc.receive(waitmsec)
  command = message[0]
  debugtext = message[1:]
  if command == pynvc.DEBUG_TRACE_PART:
    if not messages_in_progress.has_key(src):
      messages_in_progress[src] = ""
    messages_in_progress[src] += debugtext
  if command == DEBUG_TRACE_FINAL:
    if not messages_in_progress.has_key(src):
      debugtext = messages_in_progress[src] + debugtext
      messages_in_progress[src] = ""
    print "Node %d) %s" % (src, debugtext)
