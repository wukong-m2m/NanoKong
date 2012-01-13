import pyzwave


REPRG_OPEN                   = 0x10
REPRG_OPEN_R                 = 0x11
REPRG_WRITE                  = 0x20
REPRG_WRITE_R_OK             = 0x21
REPRG_WRITE_R_RETRANSMIT     = 0x22
REPRG_COMMIT                 = 0x30
REPRG_COMMIT_R_OK            = 0x31
REPRG_COMMIT_R_RETRANSMIT    = 0x32
REPRG_COMMIT_R_FAILED        = 0x33

GETRUNLVL                    = 0x50
GETRUNLVL_R                  = 0x51
SETRUNLVL                    = 0x60
SETRUNLVL_R                  = 0x61

APPMSG                       = 0x80
APPMSG_R                     = 0x81

APPMSG_STATUS_WAIT_ACK       = 0x00
APPMSG_STATUS_ACK            = 0x01
APPMSG_STATUS_BUSY           = 0x02

RUNLVL_VM                    = 0x02
RUNLVL_CONF                  = 0x03
RUNLVL_RESET                 = 0x04

def sendcmd(dest, cmd, payload=[], retries=3):
  pyzwave.receive(10) # Clear pending messages
  while retries >= 0:
    try:
      pyzwave.send(dest, [0x88, cmd] + payload)
    except:
      print "=============IOError============ retries remaining:"
      print retries
      if retries <= 0:
        raise
      else:
        retries -= 1
    else:
      if cmd == APPMSG:
        ack = pyzwave.receive(5000) # Receive ack of APPMSG, TODO: see if sending succeeded.
        print "APPMSG ACK:", ack
      return

def receive(waitmsec=1000):
  return pyzwave.receive(waitmsec)

def checkedReceive(allowedReplies, waitmsec=1000):
  reply = pyzwave.receive(waitmsec)
  if reply == None:
    print "No reply received. One of", allowedReplies, "expected."
    return None
  if not reply[0] in allowedReplies:
    print "Incorrect reply received. One of", allowedReplies, "expected, but got", reply
    return None
  return reply

def sendWithRetryAndCheckedReceive(destination, command, allowedReplies, payload=[], waitmsec=1000, retries=10, quitOnFailure=False):
  while retries >= 0:
    try:
      sendcmd(destination, command, payload)
      reply = checkedReceive(allowedReplies, waitmsec)
      if not reply == None:
        return reply
    except:
      pass
    retries -= 1
  if quitOnFailure:
    print "=====FAIL====="
    print "No reply received for command:"
    print command
    print "payload:"
    print payload
    print "Aborting"
    quit()
  else:
    return None

def sendWithRetryAndCheckedReceiveAPPMSG(destination, command, allowedReplies, payload=[], waitmsec=1000, retries=10, quitOnFailure=False):
  while retries >= 0:
    try:
      sendcmd(destination, APPMSG, [command] + payload)
      reply = checkedReceive([APPMSG], waitmsec)
      if not reply == None and len(reply) > 1 and reply[1] in allowedReplies:
        return reply[1:]
      else:
        print "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
        print reply
        print reply[1]
        print allowedReplies 
    except:
      pass
    retries -= 1
  if quitOnFailure:
    print "=====FAIL====="
    print "No reply received for command:"
    print command
    print "payload:"
    print payload
    print "Aborting"
    quit()
  else:
    return None
def init():
  pyzwave.init("10.3.36.231")