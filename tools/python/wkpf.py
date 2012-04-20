#!/usr/bin/python

import sys
import pynvc

DATATYPE_INT16 = 0
DATATYPE_BOOLEAN = 1

global __sequenceNumber
__sequenceNumber = 0

def getNextSequenceNumberAsList():
  global __sequenceNumber
  __sequenceNumber = (__sequenceNumber + 1) % (2**16)
  return [__sequenceNumber/256, __sequenceNumber%256]

class Endpoint:
  def __init__(self, nodeId, portNumber, profileId):
    self.nodeId = nodeId
    self.portNumber = portNumber
    self.profileId = profileId
  def __repr__(self):
    return 'endpoint(node %d port %d profile %d)' % (self.nodeId, self.portNumber, self.profileId)


def getProfileList(destination):
  sn = getNextSequenceNumberAsList()
  reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                command=pynvc.WKPF_GET_PROFILE_LIST,
                                                payload=sn,
                                                allowedReplies=[pynvc.WKPF_GET_PROFILE_LIST_R, pynvc.WKPF_ERROR_R],
                                                quitOnFailure=True)
  if len(reply) < 4 or reply[0:3] != [pynvc.WKPF_GET_PROFILE_LIST_R] + sn:
    print "Incorrect reply: ", reply
    return None
  profiles = []
  reply = reply[4:]
  while len(reply) > 1:
    profiles.append((reply[0] <<8) + reply[1])
    reply = reply[2:]
  return profiles

def getEndpointList(destination):
  sn = getNextSequenceNumberAsList()
  reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                command=pynvc.WKPF_GET_ENDPOINT_LIST,
                                                payload=sn,
                                                allowedReplies=[pynvc.WKPF_GET_ENDPOINT_LIST_R, pynvc.WKPF_ERROR_R],
                                                quitOnFailure=True)
  if len(reply) < 4 or reply[0:3] != ([pynvc.WKPF_GET_ENDPOINT_LIST_R] + sn):
    print "Incorrect reply: ", reply
    return None
  profiles = []
  reply = reply[4:]
  while len(reply) > 1:
    profiles.append(Endpoint(destination, reply[0], (reply[1] <<8) + reply[2]))
    reply = reply[3:]
  return profiles

def getProperty(endpoint, propertyNumber):
  sn = getNextSequenceNumberAsList()
  payload=sn+[endpoint.portNumber, endpoint.profileId/256, endpoint.profileId%256, propertyNumber]
  reply = pynvc.sendWithRetryAndCheckedReceive(destination=endpoint.nodeId,
                                                command=pynvc.WKPF_READ_PROPERTY,
                                                payload=payload,
                                                allowedReplies=[pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                quitOnFailure=True)
  if len(reply) < 9 or reply[0:7] != ([pynvc.WKPF_READ_PROPERTY_R] + payload):
    print "Incorrect reply: ", reply
    return None
  datatype = reply[7]
  if datatype == DATATYPE_BOOLEAN:
    return reply[8] != 0
  else:
    return (reply[8] <<8) + reply[9]

def setProperty(endpoint, propertyNumber, datatype, value):
  sn = getNextSequenceNumberAsList()
  if datatype == DATATYPE_BOOLEAN:
    payload=sn+[endpoint.portNumber, endpoint.profileId/256, endpoint.profileId%256, propertyNumber, datatype, 1 if value else 0]
  else:
    payload=sn+[endpoint.portNumber, endpoint.profileId/256, endpoint.profileId%256, propertyNumber, datatype, value/256, value%256]
  reply = pynvc.sendWithRetryAndCheckedReceive(destination=endpoint.nodeId,
                                                command=pynvc.WKPF_WRITE_PROPERTY,
                                                payload=payload,
                                                allowedReplies=[pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                quitOnFailure=True)
  if reply != ([pynvc.WKPF_WRITE_PROPERTY_R] + payload[0:6]):
    print ([pynvc.WKPF_WRITE_PROPERTY_R] + payload[0:6])
    print "Incorrect reply: ", reply
    return None
  return value


pynvc.init(0)
#print getProfileList(3)
#print getEndpointList(3)
#print getProperty(Endpoint(nodeId=3, portNumber=3, profileId=1), 3)
#print setProperty(Endpoint(nodeId=3, portNumber=1, profileId=3), 0, DATATYPE_INT16, 255)
