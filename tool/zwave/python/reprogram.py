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
  
def reprogramNvmdefault():
  MESSAGESIZE = 16
  pynvc3.sendcmd(pynvc3.REPRG_OPEN)
  checkedReceive(1000, [pynvc3.REPRG_OPEN_R])

  lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(sys.argv[1]).readlines() if l.startswith('0x')]
  bytecode = []
  for l in lines:
    for b in l.split():
      bytecode.append(int(b, 16))

  print "Uploading", len(bytecode), "bytes."

  pos = 0
  while not pos == len(bytecode):
    payload_pos = [pos/256, pos%256]
    payload_data = bytecode[pos:pos+MESSAGESIZE]
    pynvc3.sendcmd(pynvc3.REPRG_WRITE, payload_pos+payload_data)
    pos += len(payload_data)

    # Check if the node sending REPRG_WRITE_R_RETRANSMIT
    reply = checkedReceive(1000,[pynvc3.REPRG_WRITE_R_OK, pynvc3.REPRG_WRITE_R_RETRANSMIT])

    if reply[0] == pynvc3.REPRG_WRITE_R_RETRANSMIT:
      pos = reply[1]*256 + reply[2]
      print "Received request to retransmit from ", pos
        
    # Send REPRG_VERIFY after last packet
    if pos == len(bytecode):
      payload = [pos/256, pos%256]
      pynvc3.sendcmd(pynvc3.REPRG_COMMIT, payload)
      reply = checkedReceive(1000, [pynvc3.REPRG_WRITE_R_RETRANSMIT,
                                    pynvc3.REPRG_COMMIT_R_RETRANSMIT,
                                    pynvc3.REPRG_COMMIT_R_FAILED,
                                    pynvc3.REPRG_COMMIT_R_OK])
      if reply[0] == pynvc3.REPRG_WRITE_R_RETRANSMIT or reply[0] == pynvc3.REPRG_COMMIT_R_RETRANSMIT:
        pos = reply[1]*256 + reply[2]
        print "Received request to retransmit from ", pos
      elif reply[0] == pynvc3.REPRG_COMMIT_R_FAILED:
        print "Commit failed."
        quit()
      else:
        print reply
        print "Commit OK."
  pynvc3.sendcmd(pynvc3.SETRUNLVL, [pynvc3.RUNLVL_RESET])

pynvc3.init()
reprogramNvmdefault()
