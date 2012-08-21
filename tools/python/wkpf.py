# vim: ts=4 sw=4
#!/usr/bin/python
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "../python"))
from wkxml import WuClassXMLParser

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
        self.from = (fromWuObject, fromProperty)
        self.to = (toWuObject, toProperty)

class WuApplication:
    def __init__(self, flowDom, outputDir, componentDir):
        self.applicationDom = flowDom;
        self.applicationName = flowDom.getElementsByTagName('application')[0].getAttribute('name')
        self.destinationDir = outputDir
        self.componentDir = componentDir

    def parseComponents(self):
        componentParser = WuClassXMLParser(self.componentDir)
        self.wuClassdefs = componentParser.parse() # wuClassdefs of components

    def scaffoldingWithComponents(self):
        self.wuObjects = WuObjectDefList()
        self.wuLinks = []
        # TODO: parse application XML to generate WuClasses, WuObjects and WuLinks
        for componentTag in self.applicationDom.getElementsByTagName('component'):
            # make sure application component is found in wuClassdef component list
            assert componentTag.getAttribute('type') in self.wuClassdefs
            wuClassdef = self.wuClassdefs.getByTypeName(componentTag.getAttribute('type'))
            # nodeId is not used here, portNumber is generated later
            self.wuObjects.append( WuObject(wuClassDef=wuClassdef, instanceId=componentTag.getAttribute('instanceId')) )
            # TODO: for java variable instantiation
            for propertyTag in componentTag.getElementsByTagName('property'):
                assert propertyTag.getAttribute('name') in wuClassDef
                wuObject = self.wuObjects.getByTypeName(wuClassDef.getTypeName())
                wuProperty = wuObject.getByName(propertyTag.getAttribute('name'))
                wuProperty.default_value = propertyTag.getAttribute('default')

        # links
        for linkTag in self.applicationDom.getElementsByTagName('link'):
            fromWuObject = self.wuObjects.getByInstanceId(linkTag.parentNode().getAttribute('instanceId'))
            fromProperty = linkTag.getAttribute('fromProperty')

            toWuObject = self.wuObjects.getByInstanceId(linkTag.getAttribute('toInstanceId'))
            toProperty = linkTag.getAttribute('toProperty')

            self.wuLinks.append( WuLink(fromWuObject, fromProperty, toWuObject, toProperty) )

    def mappingWithNodeList(self, nodes):
        # TODO: mapping results for generating the appropriate instiantiation for different nodes
        pass

    def generateJava(self):
        open(os.path.join(self.destinationDir, self.applicationName+".java"), 'w')

class WuObjectDefList:
    def __init__(self):
        self.wuobjects = []

    def __repr__(self):
        return self.wuobjects

    def __contains__(self, typeName):
        return typeName in [wuobject.getXmlName() for wuobject in self.wuobjects]

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

class WuObject(WuClass):
    def __init__(self, portNumber=None, instanceId, **param):
        super(WuObject, self).__init__(param)
        self.portNumber = portNumber
        self.instanceId = instanceId
    def __repr__(self):
        return 'wuobject(node %d port %d wuclass %d)' % (self.nodeId, self.portNumber, self.wuClassId)

class WuClass:
    def __init__(self, wuClassDef, nodeId=None):
        self.nodeId = nodeId
        self.wuClassDef = wuClassDef
    def __repr__(self):
        return 'wuclass(node %d wuclass %d isvirtual %s)' % (self.nodeId, self.wuClassId, str(self.isVirtual))

class NodeInfo:
    def __init__(self, nodeId, wuClasses, wuObjects, isResponding=True):
        self.nodeId = nodeId
        self.isResponding = isResponding
        if isResponding:
            self.wuClasses = wuClasses
            self.wuObjects = wuObjects
            self.nativeWuClasses = [wuclass.wuClassId for wuclass in self.wuClasses if not wuclass.isVirtual]
            self.virtualWuClasses = [wuclass.wuClassId for wuclass in self.wuClasses if wuclass.isVirtual]
# Technically this is not 100% correct since we could make a virtual wuObject even if the native class is present, but this will work for now
            self.nativeWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.wuClassId in self.nativeWuClasses]
            self.virtualWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.wuClassId in self.virtualWuClasses]
    def __repr__(self):
        return '(nodeinfo node %d\nwuclasses %s\nnative wuclasses %s\nvirtual wuclasses %s\nwuobjects %s)\n' % (self.nodeId, str(self.wuClasses), str(self.nativeWuClasses), str(self.virtualWuClasses), str(self.wuObjects))

