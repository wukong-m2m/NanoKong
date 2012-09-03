# vim: ts=4 sw=4
#!/usr/bin/python

import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "../python"))
#sys.path.append(os.path.join(os.path.dirname(__file__), "../xml2java"))
from wkxml import WuXMLParser
from wkxml import WuClassDef


DATATYPE_INT16 = 0
DATATYPE_BOOLEAN = 1
DATATYPE_REFRESH_RATE = 2

def datatypeToString(datatype):
    if datatype == DATATYPE_INT16:
        return "INT16"
    elif datatype == DATATYPE_BOOLEAN:
        return "BOOLEAN"
    elif datatype == DATATYPE_REFRESH_RATE:
        return "REFRESH"
    else:
        raise Error('Unknown datatype %d' % (datatype))

class WuLink:
    def __init__(self, fromWuObject, fromProperty, toWuObject, toProperty):
        self.linkSrc = (fromWuObject, fromProperty)
        self.linkDst = (toWuObject, toProperty)


class WuObjectDefList:
    def __init__(self):
        self.wuobjects = []

    def __repr__(self):
        return self.wuobjects

    def __str__(self):
        return ', '.join(['Name:' + wuobjectgetXmlName() for wuobject in self.wuobjects])

    def __contains__(self, typeName):
        return typeName in [wuobject.getXmlName() for wuobject in self.wuobjects]
#Sen 8.21 override len, [] operator
    def __len__(self):
	    return len(self.wuobjects)

    def __getitem__(self, index):
        return self.wuobjects[index]

    def getByInstanceId(self, instanceId):
        for wuobject in self.wuobjects:
            if instanceId == wuobject.instanceId:
                return wuobject

    def getByTypeName(self, typeName):
        for wuobject in self.wuobjects:
            if typeName == wuobject.getXmlName():
                return wuobject

    def append(self, wuobject):
        self.wuobjects.append(wuobject)

#   12.8.21 New WuObject and WuClass class designed for backward compatibility and further usage by mapper-- Sen
#		class WuObject:
#		  def __init__(self, nodeId, portNumber, wuClassId):
#		    self.nodeId = nodeId
#		    self.portNumber = portNumber
#		    self.wuClassId = wuClassId
#		  def __repr__(self):
#		    return 'wuobject(node %d port %d wuclass %d)' % (self.nodeId, self.portNumber, self.wuClassId)
#
#		class WuClass:
#		  def __init__(self, nodeId, wuClassId, isVirtual):
#		    self.nodeId = nodeId
#		    self.wuClassId = wuClassId
#		    self.isVirtual = isVirtual
#		  def __repr__(self):
#		    return 'wuclass(node %d wuclass %d isvirtual %s)' % (self.nodeId, self.wuClassId, str(self.isVirtual))


#WuClass init definition is tricky for now..parameter isVirtual is for inspector.py, wuClassDef is for other part...wondering if there is a better way
class WuClass:
    
    def __init__(self, nodeId, wuClassId, isVirtual=None, wuClassDef = None):
        self.nodeId = nodeId
        self.wuClassId = wuClassId
        if wuClassDef == None:
            self.wuClassDef = WuClassDef(name='', id=wuClassId, properties={}, virtual=isVirtual, soft=None)
        else:
            self.wuClassDef = wuClassDef    
    def isVirtual(self):
        return self.wuClassDef.isVirtual()
    def __repr__(self):
        return 'wuclass(node %d wuclass %d)' % (self.nodeId, self.wuClassId)

class WuObject:
 #   def __init__(self, portNumber, instanceId, **param):
  #      self.wuClass = WuClass(param)
   #     self.portNumber = portNumber
    #    self.instanceId = instanceId
    def __init__(self, nodeId, portNumber, wuClassId, instanceId):
        self.wuClass = WuClass(nodeId, wuClassId)
        self.portNumber = portNumber
        self.instanceId = instanceId
    def __init__(self, portNumber, wuClass, instanceId = None):
        self.wuClass = wuClass
        self.portNumber = portNumber
        self.instanceId = instanceId
    def getWuClassId(self):
        return self.wuClass.wuClassId
    def getName(self):
        return self.wuClass.wuClassDef.getXmlName()
    def getNodeId(self):
        return self.wuClass.nodeId
    def setNodeId(self, nodeId):
        self.wuClass.nodeId = nodeId
    
    def setPortNumber(self,portNumber):
        self.portNumber = portNumber
    def getPropertyByName(self, prop_name):
        return self.wuClass.wuClassDef.getPropertyByName(prop_name)
    def getProperties(self):
        return self.wuClass.wuClassDef.getProperties()
    def __repr__(self):
        return 'wuobject(node:'+ str(self.getNodeId())+' port:'+ str(self.portNumber)+ ' wuclass: '+ str(self.getWuClassId())+')'


class NodeInfo:
    def __init__(self, nodeId, wuClasses, wuObjects, isResponding=True):
        self.nodeId = nodeId
        self.isResponding = isResponding
        if isResponding:
            self.wuClasses = wuClasses
            self.wuObjects = wuObjects
            self.nativeWuClasses = [wuclass.wuClassId for wuclass in self.wuClasses if not wuclass.isVirtual()]
            self.virtualWuClasses = [wuclass.wuClassId for wuclass in self.wuClasses if wuclass.isVirtual()]
# Technically this is not 100% correct since we could make a virtual wuObject even if the native class is present, but this will work for now
            self.nativeWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.getWuClassId() in self.nativeWuClasses]
            self.virtualWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.getWuClassId() in self.virtualWuClasses]
    def __repr__(self):
        return '(nodeinfo node %d\nwuclasses %s\nnative wuclasses %s\nvirtual wuclasses %s\nwuobjects %s)\n' % (self.nodeId, str(self.wuClasses), str(self.nativeWuClasses), str(self.virtualWuClasses), str(self.wuObjects))

