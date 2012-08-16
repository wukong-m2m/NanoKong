#!/usr/bin/python
import sys
import pynvc
from wkpf import *

HASBASESTATION = False

global __sequenceNumber
__sequenceNumber = 0

def _getNextSequenceNumberAsList():
  global __sequenceNumber
  __sequenceNumber = (__sequenceNumber + 1) % (2**16)
  return [__sequenceNumber/256, __sequenceNumber%256]

def verifyWKPFmsg(messageStart, minAdditionalBytes):
  # minPayloadLength should not include the command or the 2 byte sequence number
  return lambda command, payload: (command == pynvc.WKPF_ERROR_R) or (payload != None and payload[0:len(messageStart)]==messageStart and len(payload) >= len(messageStart)+minAdditionalBytes)

def getNodeIds():
  return pynvc.discoverNodes()

def getNodeInfos():
  nodeIds = pynvc.discoverNodes()
  return [getNodeInfo(destination) for destination in nodeIds]

def getNodeInfo(destination):
  wuClasses = getWuClassList(destination)
  wuObjects = getWuObjectList(destination)
  if wuClasses == None:
    return NodeInfo(nodeId = destination,
                    wuClasses = None,
                    wuObjects = None,
                    isResponding = False)
  else:
    return NodeInfo(nodeId = destination,
                    wuClasses = wuClasses,
                    wuObjects = wuObjects)

def getWuClassList(destination):
  sn = _getNextSequenceNumberAsList()
  src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                    command=pynvc.WKPF_GET_WUCLASS_LIST,
                                                    payload=sn,
                                                    allowedReplies=[pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R],
                                                    verify=verifyWKPFmsg(messageStart=sn, minAdditionalBytes=1)) # number of wuclasses
  if reply == None:
    return None
  if reply[0] == pynvc.WKPF_ERROR_R:
    print "WKPF RETURNED ERROR ", reply[3]
    return None
  wuclasses = []
  reply = reply[4:]
  while len(reply) > 1:
    wuClassId = (reply[0] <<8) + reply[1]
    isVirtual = True if reply[2] == 1 else False
    wuclasses.append(WuClass(destination, wuClassId, isVirtual))
    reply = reply[3:]
  return wuclasses

def getWuObjectList(destination):
  sn = _getNextSequenceNumberAsList()
  src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                    command=pynvc.WKPF_GET_WUOBJECT_LIST,
                                                    payload=sn,
                                                    allowedReplies=[pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R],
                                                    verify=verifyWKPFmsg(messageStart=sn, minAdditionalBytes=1)) # number of wuobjects
  if reply == None:
    return None
  if reply[0] == pynvc.WKPF_ERROR_R:
    print "WKPF RETURNED ERROR ", reply[3]
    return None
  wuobjects = []
  reply = reply[4:]
  while len(reply) > 1:
    wuobjects.append(WuObject(destination, reply[0], (reply[1] <<8) + reply[2]))
    reply = reply[3:]
  return wuobjects

def getProperty(wuobject, propertyNumber):
  sn = _getNextSequenceNumberAsList()
  payload=sn+[wuobject.portNumber, wuobject.wuClassId/256, wuobject.wuClassId%256, propertyNumber]
  src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=wuobject.nodeId,
                                                    command=pynvc.WKPF_READ_PROPERTY,
                                                    payload=payload,
                                                    allowedReplies=[pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                    verify=verifyWKPFmsg(messageStart=payload, minAdditionalBytes=2)) # datatype + value
  if reply == None:
    return None
  if reply[0] == pynvc.WKPF_ERROR_R:
    print "WKPF RETURNED ERROR ", reply[3]
    return None
  datatype = reply[7]
  status = reply[8]
  if datatype == DATATYPE_BOOLEAN:
    value = reply[9] != 0
  elif datatype == DATATYPE_INT16 or datatype == DATATYPE_REFRESH_RATE:
    value = (reply[9] <<8) + reply[10]
  else:
    value = None
  return (value, datatype, status)

def setProperty(wuobject, propertyNumber, datatype, value):
  sn = _getNextSequenceNumberAsList()
  if datatype == DATATYPE_BOOLEAN:
    payload=sn+[wuobject.portNumber, wuobject.wuClassId/256, wuobject.wuClassId%256, propertyNumber, datatype, 1 if value else 0]
  elif datatype == DATATYPE_INT16 or datatype == DATATYPE_REFRESH_RATE:
    payload=sn+[wuobject.portNumber, wuobject.wuClassId/256, wuobject.wuClassId%256, propertyNumber, datatype, value/256, value%256]
  src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=wuobject.nodeId,
                                                    command=pynvc.WKPF_WRITE_PROPERTY,
                                                    payload=payload,
                                                    allowedReplies=[pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                    verify=verifyWKPFmsg(messageStart=payload[0:6], minAdditionalBytes=0))
  if reply == None:
    return None
  if reply[0] == pynvc.WKPF_ERROR_R:
    print "WKPF RETURNED ERROR ", reply[3]
    return None
  return value

def init(mode, debug=False):
  pynvc.init(0, debug=debug)

#print getWuClassList(3)
#print getWuObjectList(3)
#print getProperty(WuObject(nodeId=3, portNumber=4, wuClassId=4), 0)
#print setProperty(WuObject(nodeId=3, portNumber=1, wuClassId=3), 0, DATATYPE_INT16, 255)
