#!/usr/bin/python
# vim: ts=2 sw=2
import sys
import time
from transport import *
from wkpf import *
from locationTree import *
import fakedata

communication = None

# routing services here
class Communication:
    @classmethod
    def init(cls):
      print 'Communication init'
      global communication
      if not communication:
        communication = Communication()
      return communication

    def __init__(self):
      print 'Communciation constructor'
      self.all_node_infos = []
      self.zwave = ZwaveAgent.init()
      if not self.zwave:
        print 'cannot initiate zwaveagent'
        raise Exception

    def addActiveNodesToLocTree(self, locTree):
      for node_info in self.getActiveNodeInfos():
        print 'active node_info', node_info
        locTree.addSensor(SensorNode(node_info))

    def verifyWKPFmsg(self, messageStart, minAdditionalBytes):
      # minPayloadLength should not include the command or the 2 byte sequence number
      return lambda command, payload: (command == pynvc.WKPF_ERROR_R) or (payload != None and payload[0:len(messageStart)]==messageStart and len(payload) >= len(messageStart)+minAdditionalBytes)

    def getNodeIds(self):
      return self.zwave.discovery()

    def getActiveNodeInfos(self):
      print 'getActiveNodeInfos'
      self.all_node_infos = self.getAllNodeInfos()
      return filter(lambda item: item.isResponding(), self.all_node_infos)

    def getNodeInfos(self, node_ids):
      print 'getNodeInfos', node_ids
      if self.all_node_infos:
        return filter(lambda info: info.nodeId in node_ids, self.all_node_infos)
      else:
        return [self.getNodeInfo(int(destination)) for destination in node_ids]

    def getAllNodeInfos(self, force=False):
      print 'getAllNodeInfos'
      if force or self.all_node_infos == []:
        nodeIds = self.getNodeIds()
        print 'getAllNodeInfos', nodeIds
        self.all_node_infos = [self.getNodeInfo(int(destination)) for destination in nodeIds]
      return self.all_node_infos

    def onAddMode(self):
      return self.zwave.add()

    def onDeleteMode(self):
      return self.zwave.delete()

    def onStopMode(self):
      return self.zwave.stop()
        
    def currentStatus(self):
      return self.zwave.poll()

    def getNodeInfo(self, destination):
      print 'getNodeInfo'

      wuClasses = self.getWuClassList(destination)
      print wuClasses

      wuObjects = self.getWuObjectList(destination)
      print wuObjects

      location = self.getLocation(destination)
      print location

      return NodeInfo(nodeId = destination,
                        wuClasses = wuClasses,
                        wuObjects = wuObjects,
                        location = location)

    def getLocation(self, destination):
      print 'getLocation'

      reply = self.zwave.send(destination, pynvc.WKPF_GET_LOCATION, [], [pynvc.WKPF_GET_LOCATION_R, pynvc.WKPF_ERROR_R])

      '''
      sn = self.getNextSequenceNumberAsList()
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_GET_LOCATION,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_GET_LOCATION_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn, minAdditionalBytes=0)) # number of wuclasses
      '''

      if reply == None:
        return ""

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.command
        return [] # graceful degradation

      print reply
      reply = reply.payload[2:] # without seq number
      location_size = reply[0]
      return ''.join([chr(bit) for bit in reply[1:location_size+1]]) # shift overhead

    def setLocation(self, destination, location):
      print 'setLocation'

      reply = self.zwave.send(destination, pynvc.WKPF_SET_LOCATION, [len(location)] + [int(ord(char)) for char in location], [pynvc.WKPF_SET_LOCATION_R, pynvc.WKPF_ERROR_R])
      print reply

      '''
      sn = self.getNextSequenceNumberAsList()
      sn += [len(location)] + [int(ord(char)) for char in location]
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_SET_LOCATION,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_SET_LOCATION_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn[:6], minAdditionalBytes=0)) # number of wuclasses
      '''

      if reply == None:
        return -1

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return False
      return True

    def getWuClassList(self, destination):
      print 'getWuClassList'

      reply = self.zwave.send(destination, pynvc.WKPF_GET_WUCLASS_LIST, [], [pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R])

      '''
      sn = self.getNextSequenceNumberAsList()
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_GET_WUCLASS_LIST,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn, minAdditionalBytes=1)) # number of wuclasses
      '''

      if reply == None:
        return []

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return []

      wuclasses = []
      reply = reply.payload[3:]
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
      print 'getWuObjectList'

      reply = self.zwave.send(destination, pynvc.WKPF_GET_WUOBJECT_LIST, [], [pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R])

      '''
      sn = self.getNextSequenceNumberAsList()
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.WKPF_GET_WUOBJECT_LIST,
                                                        payload=sn,
                                                        allowedReplies=[pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=sn, minAdditionalBytes=1)) # number of wuobjects
      '''

      if reply == None:
        return []

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return []

      wuobjects = []
      reply = reply.payload[3:]
      while len(reply) > 1:
        wuClassId = (reply[1] <<8) + reply[2]
        for wuclass in fakedata.all_wuclasses:
          if wuclass.getId() == wuClassId:
            wuobjects.append(WuObject(wuclass, 'testId', 1, nodeId=destination, portNumber=reply[0]))
        #wuobjects.append(WuObject(destination, reply[0], (reply[1] <<8) + reply[2]))
        reply = reply[3:]
      return wuobjects

    def getProperty(self, wuobject, propertyNumber):
      print 'getProperty'

      reply = self.zwave.send(destination, pynvc.WKPF_READ_PROPERTY, [wuobject.getPortNumber(), wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber], [pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R])

      '''
      sn = self.getNextSequenceNumberAsList()
      payload=sn+[wuobject.getPortNumber(), wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber]
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=wuobject.getNodeId(),
                                                        command=pynvc.WKPF_READ_PROPERTY,
                                                        payload=payload,
                                                        allowedReplies=[pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R],
                                                        verify=self.verifyWKPFmsg(messageStart=payload, minAdditionalBytes=2)) # datatype + value
      '''

      if reply == None:
        return (None, None, None)

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return (None, None, None)

      # compatible
      reply = [reply.command] + reply.payload

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
      print 'setProperty'

      if datatype == DATATYPE_BOOLEAN:
        payload=[wuobject.portNumber, wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber, datatype, 1 if value else 0]

      elif datatype == DATATYPE_INT16 or datatype == DATATYPE_REFRESH_RATE:
        payload=[wuobject.portNumber, wuobject.getWuClassId()/256, wuobject.getWuClassId()%256, propertyNumber, datatype, value/256, value%256]

      reply = self.zwave.send(destination, pynvc.WKPF_WRITE_PROPERTY, payload, [pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R])

      '''
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
      '''

      if reply == None:
        return None

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return None
      return value

    def reprogram(self, destination, filename, retry=False):
      print 'reprgramming'

      ret = self.reprogramNvmdefault(destination, filename)
      if retry:
        if not ret:
          print "Retrying after 5 seconds..."
          time.sleep(5)
          return self.reprogramNvmdefault(destination, filename)
      else:
        return ret

    def reprogramNvmdefault(self, destination, filename):
      MESSAGESIZE = 16

      reply = self.zwave.send(destination, pynvc.REPRG_OPEN, [], [pynvc.REPRG_OPEN_R])

      '''
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                    command=pynvc.REPRG_OPEN,
                                                    allowedReplies=[pynvc.REPRG_OPEN_R],
                                                    quitOnFailure=False)
      '''
      if reply == None:
        return None

      reply = [reply.command] + reply.payload[2:] # without the seq numbers

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
        print "Uploading bytes", pos, "to", pos+MESSAGESIZE, "of", len(bytecode)
        print pos/pagesize, (pos+len(payload_data))/pagesize, "of pagesize", pagesize
        if pos/pagesize == (pos+len(payload_data))/pagesize:
          #pynvc.sendcmd(destination, pynvc.REPRG_WRITE, payload_pos+payload_data)
          self.zwave.send(destination, pynvc.REPRG_WRITE, payload_pos+payload_data, [])
          pos += len(payload_data)
        else:
          print "Send last packet of this page and wait for a REPRG_WRITE_R_RETRANSMIT after each full page"
          reply = self.zwave.send(destination, pynvc.REPRG_WRITE, payload_pos+payload_data, [pynvc.REPRG_WRITE_R_OK, pynvc.REPRG_WRITE_R_RETRANSMIT])
          '''
          src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.REPRG_WRITE,
                                                        allowedReplies=[pynvc.REPRG_WRITE_R_OK, pynvc.REPRG_WRITE_R_RETRANSMIT],
                                                        payload=payload_pos+payload_data)
          '''
          print "Page boundary reached, wait for REPRG_WRITE_R_OK or REPRG_WRITE_R_RETRANSMIT"
          if reply.command == pynvc.REPRG_WRITE_R_OK:
            print "Received REPRG_WRITE_R_OK in reply to packet writing at", payload_pos
            pos += len(payload_data)
          elif reply.command == pynvc.REPRG_WRITE_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "===========>Received REPRG_WRITE_R_RETRANSMIT request to retransmit from ", pos
          else:
            print "No reply received. Code update failed. :-("
            return False

        if pos == len(bytecode):
          print "Send REPRG_COMMIT after last packet"
          reply = self.zwave.send(destination, pynvc.REPRG_COMMIT, [pos/256, pos%256], [pynvc.REPRG_COMMIT_R_RETRANSMIT, pynvc.REPRG_COMMIT_R_FAILED, pynvc.REPRG_COMMIT_R_OK])
          '''
          src, reply = pynvc.sendWithRetryAndCheckedReceive(
                            destination=destination,
                            command=pynvc.REPRG_COMMIT,
                            allowedReplies=[pynvc.REPRG_COMMIT_R_RETRANSMIT,
                                            pynvc.REPRG_COMMIT_R_FAILED,
                                            pynvc.REPRG_COMMIT_R_OK],
                            payload=[pos/256, pos%256])
          '''
          if reply.command == pynvc.REPRG_COMMIT_R_OK:
            print reply.payload
            print "Commit OK."
          elif reply.command == pynvc.REPRG_COMMIT_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos
          else:
            print "Commit failed."
            return False

      reply = self.zwave.send(destination, pynvc.SETRUNLVL, [pynvc.RUNLVL_RESET], [pynvc.SETRUNLVL_R])
      '''
      src, reply = pynvc.sendWithRetryAndCheckedReceive(destination=destination,
                                                        command=pynvc.SETRUNLVL,
                                                        allowedReplies=[pynvc.SETRUNLVL_R],
                                                        payload=[pynvc.RUNLVL_RESET])
      '''

      if reply == None:
        print "Going to runlevel reset failed. :-("
        return False;
      else:
        return True;

def getComm():
  return Communication.init()

#print getWuClassList(3)
#print getWuObjectList(3)
#print getProperty(WuObject(nodeId=3, portNumber=4, wuClassId=4), 0)
#print setProperty(WuObject(nodeId=3, portNumber=1, wuClassId=3), 0, DATATYPE_INT16, 255)
