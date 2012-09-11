#!/usr/bin/python
# vim: ts=2 sw=2
import sys
import pynvc
from wkpf import *
from locationTree import *
import fakedata

class Communication:
    def __init__(self, option):
      if not pynvc.init(option):
        raise Exception
      self.sequenceNumber = 0
      self.mode = 'stop'
      self.all_node_infos = []

    def addActiveNodesToLocTree(self, locTree):
      for node_info in self.getActiveNodeInfos():
        print 'active node_info', node_info
        locTree.addSensor(SensorNode(node_info))

    def getNextSequenceNumberAsList(self):
      self.sequenceNumber = (self.sequenceNumber + 1) % (2**16)
      return [self.sequenceNumber/256, self.sequenceNumber%256]

    def verifyWKPFmsg(self, messageStart, minAdditionalBytes):
      # minPayloadLength should not include the command or the 2 byte sequence number
      return lambda command, payload: (command == pynvc.WKPF_ERROR_R) or (payload != None and payload[0:len(messageStart)]==messageStart and len(payload) >= len(messageStart)+minAdditionalBytes)

    def getNodeIds(self):
      return pynvc.discoverNodes()

    def getActiveNodeInfos(self):
      print 'getActiveNodeInfos'
      self.all_node_infos = self.getAllNodeInfos()
      return filter(lambda item: item.isResponding(), self.all_node_infos)

    def getNodeInfos(self, node_ids):
      print 'getNodeInfos', node_ids
      return [self.getNodeInfo(int(destination)) for destination in node_ids]

    def getAllNodeInfos(self, force=False):
      print 'getAllNodeInfos'
      if force or self.all_node_infos == []:
        nodeIds = self.getNodeIds()
        print 'getAllNodeInfos', nodeIds
        self.all_node_infos = [self.getNodeInfo(int(destination)) for destination in nodeIds]
      return self.all_node_infos

    def onAddMode(self):
      if self.mode != 'stop':
        print 'need to get into stop mode first'
        return -1
      self.mode = 'add'
      return pynvc.add()

    def onDeleteMode(self):
      if self.mode != 'stop':
        print 'need to get into stop mode first'
        return -1
      self.mode = 'delete'
      return pynvc.delete()

    def onStopMode(self):
      print 'stopped'
      self.mode = 'stop'
      return pynvc.stop()
        
    def currentStatus(self):
      return pynvc.poll()

    def getNodeInfo(self, destination):
      print 'getNodeInfo'
      print 'getWuClassList'
      wuClasses = self.getWuClassList(destination)
      print wuClasses

      print 'getWuObjectList'
      wuObjects = self.getWuObjectList(destination)
      print wuObjects

      print 'getLocation'
      location = self.getLocation(destination)
      print location

      return NodeInfo(nodeId = destination,
                        wuClasses = wuClasses,
                        wuObjects = wuObjects,
                        location = location)

    def getLocation(self, destination):
      sn = self.getNextSequenceNumberAsList()
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_GET_LOCATION,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_GET_LOCATION_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn, minAdditionalBytes=0)) # number of wuclasses
      if reply == None:
        return ""
      if reply[0] == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply[3]
        return []
      print reply
      location_size = reply[3]
      return ''.join([chr(bit) for bit in reply[4:location_size+4]]) # shift overhead

    def setLocation(self, destination, location):
      sn = self.getNextSequenceNumberAsList()
      sn += [len(location)] + [int(ord(char)) for char in location]
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_SET_LOCATION,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_SET_LOCATION_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn[:6], minAdditionalBytes=0)) # number of wuclasses
      if reply == None:
        return -1
      if reply[0] == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply[3]
        return -1
      return 0

    def getWuClassList(self, destination):
      sn = self.getNextSequenceNumberAsList()
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_GET_WUCLASS_LIST,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn, minAdditionalBytes=1)) # number of wuclasses
      if reply == None:
        return []
      if reply[0] == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply[3]
        return []
      wuclasses = []
      reply = reply[4:]
      while len(reply) > 1:
        wuClassId = (reply[0] <<8) + reply[1]
        isVirtual = True if reply[2] == 1 else False
        for wuclass in fakedata.all_wuclasses:
            if wuclass.getId() == wuClassId:
                wuclass.setNodeId(destination)
                wuclasses.append(wuclass)
        #wuclasses.append(WuClass(destination, wuClassId, isVirtual))
        reply = reply[3:]
      return wuclasses

    def getWuObjectList(self, destination):
      sn = self.getNextSequenceNumberAsList()
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_GET_WUOBJECT_LIST,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn, minAdditionalBytes=1)) # number of wuobjects
      if reply == None:
        return []
      if reply[0] == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply[3]
        return []
      wuobjects = []
      reply = reply[4:]
      while len(reply) > 1:
        wuClassId = (reply[1] <<8) + reply[2]
        for wuclass in fakedata.all_wuclasses:
          if wuclass.getId() == wuClassId:
            wuobjects.append(WuObject(wuclass, 'testId', 1, nodeId=destination, portNumber=reply[0]))
        #wuobjects.append(WuObject(destination, reply[0], (reply[1] <<8) + reply[2]))
        reply = reply[3:]
      return wuobjects

    def getProperty(self, wuobject, propertyNumber):
      sn = self.getNextSequenceNumberAsList()
      payload=sn+[wuobject.getPortNumber(), wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber]
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=wuobject.getNodeId(),
                                                        command=pynvc.WKPF_READ_PROPERTY,
                                                        payload=payload,
                                                        allowedReplies=[pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=payload, minAdditionalBytes=2)) # datatype + value
      if reply == None:
        return (None, None, None)
      elif reply[0] == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply[3]
        return (None, None, None)

      datatype = reply[7]
      status = reply[8]
      if datatype == DATATYPE_BOOLEAN:
        value = reply[9] != 0
      elif datatype == DATATYPE_INT16 or datatype == DATATYPE_REFRESH_RATE:
        value = (reply[9] <<8) + reply[10]
      else:
        value = None
      return (value, datatype, status)

    def setProperty(self, wuobject, propertyNumber, datatype, value):
      sn = self.getNextSequenceNumberAsList()
      if datatype == DATATYPE_BOOLEAN:
        payload=sn+[wuobject.portNumber, wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber, datatype, 1 if value else 0]
      elif datatype == DATATYPE_INT16 or datatype == DATATYPE_REFRESH_RATE:
        payload=sn+[wuobject.portNumber, wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber, datatype, value/256, value%256]
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=wuobject.getNodeId(),
                                                        command=pynvc.WKPF_WRITE_PROPERTY,
                                                        payload=payload,
                                                        allowedReplies=[pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=payload[0:6], minAdditionalBytes=0))
      if reply == None:
        return None
      if reply[0] == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply[3]
        return None
      return value

#print getWuClassList(3)
#print getWuObjectList(3)
#print getProperty(WuObject(nodeId=3, portNumber=4, wuClassId=4), 0)
#print setProperty(WuObject(nodeId=3, portNumber=1, wuClassId=3), 0, DATATYPE_INT16, 255)
