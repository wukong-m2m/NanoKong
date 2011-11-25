#!/usr/bin/python

import sys
import pynvc3

def checkedReceive(waitmsec, allowedTypes):
  reply = pynvc3.receive(1000)
  if reply == None:
    print "No reply received. One of", allowedTypes, "expected."
    quit()
  if not reply[0] in allowedTypes:
    print "Incorrect reply received. One of", allowedTypes, "expected, but got", reply
    quit()
  return reply
  
def reprogramNvmdefault(destination, filename):
  MESSAGESIZE = 16
  pynvc3.sendcmd(destination, pynvc3.REPRG_OPEN)
  reply = checkedReceive(1000, [pynvc3.REPRG_OPEN_R])
  pagesize = reply[1]*256 + reply[2]

  lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(filename).readlines() if l.startswith('0x')]
  bytecode = []
  for l in lines:
    for b in l.split():
      bytecode.append(int(b, 16))

  print "Uploading", len(bytecode), "bytes."

  pos = 0
  while not pos == len(bytecode):
    payload_pos = [pos/256, pos%256]
    payload_data = bytecode[pos:pos+MESSAGESIZE]
    pynvc3.sendcmd(destination, pynvc3.REPRG_WRITE, payload_pos+payload_data)
    if pos/pagesize == (pos+len(payload_data))/pagesize:
      pos += len(payload_data)
    else:
      # Wait for a REPRG_WRITE_R_RETRANSMIT after each full page
      print "Page boundary reached, wait for REPRG_WRITE_R_OK or REPRG_WRITE_R_RETRANSMIT"
      reply = checkedReceive(1000,[pynvc3.REPRG_WRITE_R_OK, pynvc3.REPRG_WRITE_R_RETRANSMIT])
      if reply[0] == pynvc3.REPRG_WRITE_R_OK:
        print "Received REPRG_WRITE_R_OK in reply to packet writing at", payload_pos
        pos += len(payload_data)
      else:
        print "Received REPRG_WRITE_R_RETRANSMIT request to retransmit from ", pos
        pos = reply[1]*256 + reply[2]

    # Send REPRG_VERIFY after last packet
    if pos == len(bytecode):
      payload = [pos/256, pos%256]
      pynvc3.sendcmd(destination, pynvc3.REPRG_COMMIT, payload)
      reply = checkedReceive(1000, [pynvc3.REPRG_COMMIT_R_RETRANSMIT,
                                    pynvc3.REPRG_COMMIT_R_FAILED,
                                    pynvc3.REPRG_COMMIT_R_OK])
      if reply[0] == pynvc3.REPRG_COMMIT_R_OK:
        print reply
        print "Commit OK."
      elif reply[0] == pynvc3.REPRG_COMMIT_R_RETRANSMIT:
        pos = reply[1]*256 + reply[2]
        print "Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos
      else:
        print "Commit failed."
        quit()
  pynvc3.sendcmd(destination, pynvc3.SETRUNLVL, [pynvc3.RUNLVL_RESET])

pynvc3.init()
reprogramNvmdefault(int(sys.argv[1]), sys.argv[2])
