#!/usr/bin/python
import sys

DATATYPE_INT16 = 0
DATATYPE_BOOLEAN = 1
DATATYPE_REFRESH_RATE = 2

class WuObject:
  def __init__(self, nodeId, portNumber, wuClassId):
    self.nodeId = nodeId
    self.portNumber = portNumber
    self.wuClassId = wuClassId
  def __repr__(self):
    return 'wuobject(node %d port %d wuclass %d)' % (self.nodeId, self.portNumber, self.wuClassId)

class WuClass:
  def __init__(self, nodeId, wuClassId, isVirtual):
    self.nodeId = nodeId
    self.wuClassId = wuClassId
    self.isVirtual = isVirtual
  def __repr__(self):
    return 'wuclass(node %d wuclass %d isvirtual %s)' % (self.nodeId, self.wuClassId, str(self.isVirtual))

class NodeInfo:
    def __init__(self, nodeId, wuClasses, wuObjects):
        self.nodeId = nodeId
        self.wuClasses = wuClasses
        self.wuObjects = wuObjects
        self.nativeWuClasses = [wuclass.wuClassId for wuclass in self.wuClasses if not wuclass.isVirtual]
        self.virtualWuClasses = [wuclass.wuClassId for wuclass in self.wuClasses if wuclass.isVirtual]
        # Technically this is not 100% correct since we could make a virtual wuObject even if the native class is present, but this will work for now
        self.nativeWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.wuClassId in self.nativeWuClasses]
        self.virtualWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.wuClassId in self.virtualWuClasses]
    def __repr__(self):
        return '(nodeinfo node %d\nwuclasses %s\nnative wuclasses %s\nvirtual wuclasses %s\nwuobjects %s)\n' % (self.nodeId, str(self.wuClasses), str(self.nativeWuClasses), str(self.virtualWuClasses), str(self.wuObjects))

