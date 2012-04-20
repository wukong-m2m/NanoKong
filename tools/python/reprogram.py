#!/usr/bin/python

import sys
import pynvc

def reprogramNvmdefault(destination, filename):
  MESSAGESIZE = 16
  reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                command=pynvc.REPRG_OPEN,
                                                allowedReplies=[pynvc.REPRG_OPEN_R],
                                                quitOnFailure=True)
  pagesize = reply[1]*256 + reply[2]

  lines = [" " + l.replace('0x','').replace(',','').replace('\n','') for l in open(filename).readlines() if l.startswith('0x')]
  bytecode = []
  for l in lines:
    for b in l.split():
      bytecode.append(int(b, 16))

  print "Uploading", len(bytecode), "bytes."

  packetLost = False

  pos = 0
  while not pos == len(bytecode):
    payload_pos = [pos/256, pos%256]
    payload_data = bytecode[pos:pos+MESSAGESIZE]
    if pos/pagesize == (pos+len(payload_data))/pagesize:
      if packetLost == False and pos == 32:
        print "------------->Simulating packet loss"
        # drop this one packet
        packetLost = True
      else:
        pynvc.sendcmd(destination, pynvc.REPRG_WRITE, payload_pos+payload_data)
      pos += len(payload_data)
    else:
      # Send last packet of this page and wait for a REPRG_WRITE_R_RETRANSMIT after each full page
      reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                    command=pynvc.REPRG_WRITE,
                                                    allowedReplies=[pynvc.REPRG_WRITE_R_OK, pynvc.REPRG_WRITE_R_RETRANSMIT],
                                                    payload=payload_pos+payload_data,
                                                    quitOnFailure=True)
      print "Page boundary reached, wait for REPRG_WRITE_R_OK or REPRG_WRITE_R_RETRANSMIT"
      if reply[0] == pynvc.REPRG_WRITE_R_OK:
        print "Received REPRG_WRITE_R_OK in reply to packet writing at", payload_pos
        pos += len(payload_data)
      else:
        pos = reply[1]*256 + reply[2]
        print "===========>Received REPRG_WRITE_R_RETRANSMIT request to retransmit from ", pos

    # Send REPRG_COMMIT after last packet
    if pos == len(bytecode):
      reply = pynvc.sendWithRetryAndCheckedReceive(
                        destination=destination,
                        command=pynvc.REPRG_COMMIT,
                        allowedReplies=[pynvc.REPRG_COMMIT_R_RETRANSMIT,
                                        pynvc.REPRG_COMMIT_R_FAILED,
                                        pynvc.REPRG_COMMIT_R_OK],
                        payload=[pos/256, pos%256],
                        quitOnFailure=True)
      if reply[0] == pynvc.REPRG_COMMIT_R_OK:
        print reply
        print "Commit OK."
      elif reply[0] == pynvc.REPRG_COMMIT_R_RETRANSMIT:
        pos = reply[1]*256 + reply[2]
        print "===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos
      else:
        print "Commit failed."
        quit()
  pynvc.sendcmd(destination, pynvc.SETRUNLVL, [pynvc.RUNLVL_RESET])

if __name__ == "__main__":
  pynvc.init(1) # 0: zwave, 1: zigbee
  reprogramNvmdefault(int(sys.argv[1]), sys.argv[2])
