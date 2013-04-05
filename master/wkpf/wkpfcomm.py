import sys, time, copy
from wkpf.transport import *
from wkpf.locationTree import *
from wkpf.models import *
from wkpf.globals import *
from wkpf.configuration import *

# routing services here
class Communication:
    _communication = None
    @classmethod
    def init(cls):
      if not cls._communication:
        cls._communication = Communication()
      return cls._communication

    def __init__(self):
      self.all_node_infos = []
      self.broker = getAgent()
      self.zwave = getZwaveAgent()
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

    def getActiveNodeInfos(self, force=False):
      logging.info('getActiveNodeInfos 2')

      set_wukong_status("Discovery: Requesting node info")

      self.all_node_infos = self.getAllNodeInfos(force=force)

      set_wukong_status("")
      return filter(lambda item: item.isResponding(), self.all_node_infos)

    def getNodeInfos(self, node_ids):
      print 'getNodeInfos', node_ids
      if self.all_node_infos:
        return filter(lambda info: info.id in node_ids, self.all_node_infos)
      else:
        return [self.getNodeInfo(int(destination)) for destination in node_ids]

    def getAllNodeInfos(self, force=False):
      if force or self.all_node_infos == []:
        nodeIds = self.getNodeIds()
        self.all_node_infos = [self.getNodeInfo(int(destination)) for destination in nodeIds]
      print 'got all nodeInfos'
      return self.all_node_infos

    def getRoutingInformation(self):
      return self.zwave.routing()

    def onAddMode(self):
      return self.zwave.add()

    def onDeleteMode(self):
      return self.zwave.delete()

    def onStopMode(self):
      return self.zwave.stop()
        
    def currentStatus(self):
      return self.zwave.poll()

    def getNodeInfo(self, destination):
      print 'getNodeInfo of node id', destination

      location = self.getLocation(destination)
      print 'got location'

      wuClasses = self.getWuClassList(destination)
      print 'got wuclasses'

      wuObjects = self.getWuObjectList(destination)
      print 'got wuobjects'

      print 'create node'
      node = Node(destination, location, wuClasses, wuObjects)
      print 'preare node save'
      node.save()
      print 'node saved'
      return node

    def getLocation(self, destination):
      print 'getLocation', destination

      length = 0
      location = ''

      while (length == 0 or len(location) < length): # There's more to the location string, so send more messages to get the rest
        # +1 because the first byte in the data stored on the node is the location string length
        offset = len(location) + 1 if length > 0 else 0
        reply = self.zwave.send(destination, pynvc.WKPF_GET_LOCATION, [offset], [pynvc.WKPF_GET_LOCATION_R, pynvc.WKPF_ERROR_R])

        if reply == None:
          return ''
        if reply.command == pynvc.WKPF_ERROR_R:
          print "WKPF RETURNED ERROR ", reply.command
          return '' # graceful degradation
        if len(reply.payload) <= 2:
          return ''

        if length == 0:
          length = reply.payload[2] # byte 3 in the first message is the total length of the string
          if length == 0:
            return ''
          location = ''.join([chr(byte) for byte in reply.payload[3:]])
        else:
          location += ''.join([chr(byte) for byte in reply.payload[2:]])

      return location[0:length] # The node currently send a bit too much, so we have to truncate the string to the length we need

    def setLocation(self, destination, location):
      print 'setLocation', destination

      # Put length in front of location
      locationstring = [len(location)] + [int(ord(char)) for char in location]
      offset = 0
      chunksize = 10
      while offset < len(locationstring):
        chunk = locationstring[offset:offset+chunksize]
        message = [offset, len(chunk)] + chunk
        offset += chunksize

        reply = self.zwave.send(destination, pynvc.WKPF_SET_LOCATION, message, [pynvc.WKPF_SET_LOCATION_R, pynvc.WKPF_ERROR_R])

        if reply == None:
          return -1

        if reply.command == pynvc.WKPF_ERROR_R:
          print "WKPF RETURNED ERROR ", reply.payload
          return False
      return True

    def getFeatures(self, destination):
      print 'getFeatures'

      reply = self.zwave.send(destination, pynvc.WKPF_GET_FEATURES, [], [pynvc.WKPF_GET_FEATURES_R, pynvc.WKPF_ERROR_R])


      if reply == None:
        return ""

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.command
        return [] # graceful degradation

      print reply
      return reply[3:]

    def setFeature(self, destination, feature, onOff):
      print 'setFeature'

      reply = self.zwave.send(destination, pynvc.WKPF_SET_FEATURE, [feature, onOff], [pynvc.WKPF_SET_FEATURE_R, pynvc.WKPF_ERROR_R])
      print reply


      if reply == None:
        return -1

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return False
      return True

    def getWuClassList(self, destination):
      print 'getWuClassList'

      set_wukong_status("Discovery: Requesting wuclass list from node %d" % (destination))

      reply = self.zwave.send(destination, pynvc.WKPF_GET_WUCLASS_LIST, [], [pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R])

      print 'Respond received'

      if reply == None:
        return []

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return []

      wuclasses = []
      reply = reply.payload[3:]
      print repr(reply)
      while len(reply) > 1:
        wuClassId = (reply[0] <<8) + reply[1]
        isVirtual = True if reply[2] == 1 else False
        wuclass = None
        print 'begin wuclass_query'
        wuclass_query = WuClass.where(node_id=destination, id=wuClassId)
        print 'end wuclass_query', wuclass_query
        if not wuclass_query:
            print 'begin wuclass_component'
            wuclass_component = WuClass.where(id=wuClassId)[0]
            print 'end wuclass_component', wuclass_component
            if wuclass_component:
                new_properties_with_node_infos = copy.deepcopy(wuclass_component.properties)
                def anew(x):
                  x.identity = None # so they will be saved with new row ids
                  return x
                new_properties_with_node_infos = map(anew, new_properties_with_node_infos)
                wuclass = WuClass(wuclass_component.id, wuclass_component.name,
                        wuclass_component.virtual, wuclass_component.type,
                        new_properties_with_node_infos, destination)
                print 'prepare save wuclass'
                wuclass.save()
                print 'done save wuclass'
            else:
                print 'Unknown wuclass id', wuClassId
        else:
            wuclass = wuclass_query[0]

        if wuclass:
            wuclasses.append(wuclass)
        reply = reply[3:]
      return wuclasses

    def getWuObjectList(self, destination):
      print 'getWuObjectList'

      set_wukong_status("Discovery: Requesting wuobject list from node %d" % (destination))

      reply = self.zwave.send(destination, pynvc.WKPF_GET_WUOBJECT_LIST, [], [pynvc.WKPF_GET_WUOBJECT_LIST_R, pynvc.WKPF_ERROR_R])

      print 'Respond received'


      if reply == None:
        return []

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return []

      wuobjects = []
      reply = reply.payload[3:]
      print repr(reply)
      while len(reply) > 1:
        wuClassId = (reply[1] <<8) + reply[2]
        port_number = reply[0]
        wuobject = None
        print 'begin wuobject_query'
        wuobject_query = WuObject.where(node_id=destination, wuclass_id=wuClassId)
        print 'end wuobject_query', wuobject_query
        if wuobject_query == []:
            print 'begin wuclass_query'
            wuclass_query = WuClass.where(id=wuClassId, node_id=destination)
            print 'end wuclass_query', wuclass_query
            if len(wuclass_query) > 0:
                wuclass = wuclass_query[0]
                wuobject = WuObject(destination, port_number, wuclass)
                print 'begin save'
                wuobject.save()
                print 'end save'
            else:
                print 'Unknown wuclass id', wuClassId
        else:
            # might need to update
            wuobject = wuobject_query[0]
            wuobject.port_number = port_number
            wuobject.save()

        if wuobject:
            wuobjects.append(wuobject)
        reply = reply[3:]
      return wuobjects

    def getProperty(self, wuobject, propertyNumber):
      print 'getProperty'

      reply = self.zwave.send(wuobject.node_id, 
              pynvc.WKPF_READ_PROPERTY,
              [wuobject.port_number, wuobject.wuclass.id/256, 
                    wuobject.wuclass.id%256, propertyNumber], 
              [pynvc.WKPF_READ_PROPERTY_R, pynvc.WKPF_ERROR_R])


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
      master_busy()

      if datatype == DATATYPE_BOOLEAN:
        payload=[wuobject.port_number, wuobject.wuclass.id/256,
        wuobject.wuclass.id%256, propertyNumber, datatype, 1 if value else 0]

      elif datatype == DATATYPE_INT16 or datatype == DATATYPE_REFRESH_RATE:
        payload=[wuobject.port_number, wuobject.wuclass.id/256,
        wuobject.wuclass.id%256, propertyNumber, datatype, value/256, value%256]

      reply = self.zwave.send(wuobject.node_id, pynvc.WKPF_WRITE_PROPERTY, payload, [pynvc.WKPF_WRITE_PROPERTY_R, pynvc.WKPF_ERROR_R])


      master_available()
      if reply == None:
        return None

      if reply.command == pynvc.WKPF_ERROR_R:
        print "WKPF RETURNED ERROR ", reply.payload
        return None
      master_available()
      return value

    def reprogram(self, destination, filename, retry=1):
      master_busy()
      
      if retry < 0:
        retry = 1

      ret = self.reprogramNvmdefault(destination, filename)
      while retry and not ret:
        print "Retrying after 5 seconds..."
        time.sleep(5)
        ret = self.reprogramNvmdefault(destination, filename)
        retry -= 1
      master_available()
      return ret

    def reprogramNvmdefault(self, destination, filename):
      print "Reprogramming Nvmdefault..."
      MESSAGESIZE = 16

      reply = self.zwave.send(destination, pynvc.REPRG_OPEN, [], [pynvc.REPRG_OPEN_R])

      if reply == None:
        print "No reply, abort"
        return False

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
          print "Page boundary reached, wait for REPRG_WRITE_R_OK or REPRG_WRITE_R_RETRANSMIT"
          if reply == None:
            print "No reply received. Code update failed. :-("
            return False
          elif reply.command == pynvc.REPRG_WRITE_R_OK:
            print "Received REPRG_WRITE_R_OK in reply to packet writing at", payload_pos
            pos += len(payload_data)
          elif reply.command == pynvc.REPRG_WRITE_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "===========>Received REPRG_WRITE_R_RETRANSMIT request to retransmit from ", pos

        if pos == len(bytecode):
          print "Send REPRG_COMMIT after last packet"
          reply = self.zwave.send(destination, pynvc.REPRG_COMMIT, [pos/256, pos%256], [pynvc.REPRG_COMMIT_R_RETRANSMIT, pynvc.REPRG_COMMIT_R_FAILED, pynvc.REPRG_COMMIT_R_OK])
          if reply == None:
            print "Commit failed."
            return False
          elif reply.command == pynvc.REPRG_COMMIT_R_OK:
            print reply.payload
            print "Commit OK."
          elif reply.command == pynvc.REPRG_COMMIT_R_RETRANSMIT:
            reply = [reply.command] + reply.payload[2:] # without the seq numbers
            pos = reply[1]*256 + reply[2]
            print "===========>Received REPRG_COMMIT_R_RETRANSMIT request to retransmit from ", pos

      reply = self.zwave.send(destination, pynvc.SETRUNLVL, [pynvc.RUNLVL_RESET], [pynvc.SETRUNLVL_R])

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
