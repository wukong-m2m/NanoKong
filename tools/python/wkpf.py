# vim: ts=4 sw=4
#!/usr/bin/python

import os
import sys

from struct import pack

from jinja2 import Template
from jinja2 import Environment, FileSystemLoader

sys.path.append(os.path.join(os.path.dirname(__file__), "../python"))
#sys.path.append(os.path.join(os.path.dirname(__file__), "../xml2java"))
from wkxml import WuClassXMLParser
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

def toByteString(i): 
    return ['(byte)' + str(ord(b)) for b in pack("H", i)]

class WuLink:
    def __init__(self, fromWuObject, fromProperty, toWuObject, toProperty):
        self.linkSrc = (fromWuObject, fromProperty)
        self.linkDst = (toWuObject, toProperty)

    def toJava(self):
        return ', '.join(toByteString(int(self.linkSrc[0].instanceIndex)) + toByteString(int(self.linkSrc[1])) + toByteString(int(self.linkDst[0].instanceIndex)) + toByteString(int(self.linkDst[1])) + toByteString(int(self.linkDst[0].getWuClassId())))


class WuObjectDefList:
    def __init__(self):
        self.wuobjects = []

    def __repr__(self):
        return self.wuobjects

    def __contains__(self, typeName):
        return typeName in [wuobject.getXmlName() for wuobject in self.wuobjects]
#Sen 8.21 override len, [] operator
    def __len__(self):
	    return len(self.wuobjects)

    def __getitem__(self, index):
        return self.wuobjects[index]

    def getByInstanceId(self, instanceId):
        print 'getByInstanceId'
        print instanceId
        for wuobject in self.wuobjects:
            if instanceId == wuobject.instanceId:
                print wuobject
                return wuobject
            else:
                return None

    def getByTypeName(self, typeName):
        for wuobject in self.wuobjects:
            if typeName == wuobject.getXmlName():
                return wuobject

    def append(self, wuobject):
        wuobject.instanceIndex = len(self.wuobjects)
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
    def __init__(self, portNumber, instanceId, instanceIndex = None, **param):
        self.wuClass = WuClass(param)
        self.portNumber = portNumber
        self.instanceId = instanceId
        self.instanceIndex = instanceIndex
    def __init__(self, nodeId, portNumber, wuClassId, instanceIndex = None, instanceId = None):
        self.instanceIndex = instanceIndex
        self.wuClass = WuClass(nodeId, wuClassId)
        self.portNumber = portNumber
        self.instanceId = instanceId
        self.instanceIndex = instanceIndex
    def __init__(self, portNumber, wuClass, instanceIndex = None, instanceId = None):
        self.wuClass = wuClass
        self.portNumber = portNumber
        self.instanceId = instanceId
        self.instanceIndex = instanceIndex
    def getWuClassId(self):
        return self.wuClass.wuClassId
    def getNodeId(self):
        return self.wuClass.nodeId
    def setNodeId(self, nodeId):
        self.wuClass.nodeId = nodeId
    
    def setPortNumber(self,portNumber):
        self.portNumber = portNumber
    def getPropertyByName(self, prop_name):
        return self.wuClass.wuClassDef.getPropertyByName(prop_name)
    def toJava(self):
        print self.instanceId
        print self.getNodeId(), self.portNumber
        return ', '.join([int(self.getNodeId()), int(self.portNumber)])
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

from locationTree import LocationTree
from URLParser import *
class WuApplication:
    def __init__(self, flowDom, outputDir, componentDir, rootDir):
        self.applicationDom = flowDom;
        self.applicationName = flowDom.getElementsByTagName('application')[0].getAttribute('name')
        self.destinationDir = outputDir
        self.componentDir = componentDir
        self.templateDir = os.path.join(rootDir, 'tools', 'xml2java')
        self.wuClassDefs = None
        self.wuObjectList = WuObjectDefList()
        self.wuLinks = []

    def parseComponents(self):
        componentParser = WuClassXMLParser(self.componentDir)
        self.wuClassDefs = componentParser.parse() # wuClassDefs of components

    def scaffoldingWithComponents(self):
        # TODO: parse application XML to generate WuClasses, WuObjects and WuLinks
        wuObj = None
        for componentTag in self.applicationDom.getElementsByTagName('component'):
            # make sure application component is found in wuClassDef component list
 #           assert componentTag.getAttribute('type') in self.wuClassDefs   #Sen12.8.22 assertion type not compatible, need revision

            wuClassDef = self.wuClassDefs.getByTypeName(componentTag.getAttribute('type'))
            assert wuClassDef != None
            # nodeId is not used here, portNumber is generated later
            wuObj = WuObject(None, WuClass(None,wuClassDef.id, wuClassDef = wuClassDef), instanceId=componentTag.getAttribute('instanceId'))
            print wuObj.wuClass.wuClassDef.properties
            # TODO: for java variable instantiation
            for propertyTag in componentTag.getElementsByTagName('property'):
                assert propertyTag.getAttribute('name') in wuClassDef
               # wuObject = self.wuObjectList.getByTypeName(wuClassDef.getTypeName())
                wuProperty = wuObj.getPropertyByName(propertyTag.getAttribute('name'))
                wuProperty.default_value = propertyTag.getAttribute('default')
                print wuProperty
            self.wuObjectList.append(wuObj)

        # links
        for linkTag in self.applicationDom.getElementsByTagName('link'):
            fromWuObject = self.wuObjectList.getByInstanceId(linkTag.parentNode.getAttribute('instanceId'))
            fromProperty = fromWuObject.getPropertyByName(str(linkTag.getAttribute('fromProperty'))).getId()

            toWuObject = self.wuObjectList.getByInstanceId(str(linkTag.getAttribute('toInstanceId')))
            toProperty = toWuObject.getPropertyByName(str(linkTag.getAttribute('toProperty'))).getId()

            self.wuLinks.append( WuLink(fromWuObject, fromProperty, toWuObject, toProperty) )
    def firstCandidate(app, locTree, queries):
        #input: nodes, WuObjects, WuLinks, WuClassDefs
        #output: assign node id to WuObjects
        # TODO: mapping results for generating the appropriate instiantiation for different nodes
        candidateSets = []
        for i in range(len(app.wuObjectList)):
            if queries[i] == None:
                locURLHandler = LocationURL(None, locTree)
                candidateSets.append(locTree.root.getAllNodes())
                continue
            locURLHandler = LocationURL(queries[i], locTree)
            locURLHandler.parseURL()
            
            tmpSet = locURLHandler.solveParseTree(locURLHandler.parseTreeRoot)
            if len(tmpSet) >0:
                candidateSets.append(tmpSet)
            else:
                print 'Conditions for component '+str(len(candidateSets))+'(start form 0) too strict, no available candidate found'
                return False
        for i in range(len(app.wuObjectList)):
            app.wuObjectList[i].setNodeId(tuple(candidateSets[i])[0])    #select the first candidate who satisfies the condiditon
            sensorNode = locTree.sensor_dict[tuple(candidateSets[i])[0]]
            sensorNode.initPortList(forceInit = False)
            portNo = sensorNode.reserveNextPort()
            if portNo == None:
                print 'all port in node', i, 'occupied, cannot assign new port'
                return False
            app.wuObjectList[i].setPortNumber(portNo)
                
        return True

    def mappingWithNodeList(self, locTree, queries,mapFunc = firstCandidate):
        #input: nodes, WuObjects, WuLinks, WuClassDefs
        #output: assign node id to WuObjects
        # TODO: mapping results for generating the appropriate instiantiation for different nodes
        ret = mapFunc(self, locTree, queries)
        assert ret==True

    def generateJava(self):
        print 'inside generate Java'
        print self.wuObjectList.wuobjects
        jinja2_env = Environment(loader=FileSystemLoader([os.path.join(self.templateDir, 'jinja_templates')]))
        output = open(os.path.join(self.destinationDir, self.applicationName+".java"), 'w')
        jinja2_env.get_template('application.java').render(applicationName=self.applicationName, wuObjectList=self.wuObjectList, wuLinks=self.wuLinks)
        output.write(jinja2_env.get_template('application.java').render(applicationName=self.applicationName, wuObjectList=self.wuObjectList, wuLinks=self.wuLinks))
        output.close()
