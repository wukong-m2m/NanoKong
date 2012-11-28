# vim: ts=4 sw=4
#!/usr/bin/python

import os
import sys
import re
import xml.dom.minidom
from struct import pack
sys.path.append(os.path.join(os.path.dirname(__file__), "../python"))

DATATYPE_INT16 = 0
DATATYPE_BOOLEAN = 1
DATATYPE_REFRESH_RATE = 2

class Convert:
    @staticmethod
    def CamelCase_to_underscore(name):
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
        return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

    @staticmethod
    def underscore_to_CamelCase(name):
        return ''.join(x.capitalize() or '_' for x in name.split('_'))

    @staticmethod
    def to_c(raw):
        if re.search('.*_.*', raw) == None:
            return Convert.CamelCase_to_underscore(raw)
        else:
            return raw.lower()

    @staticmethod
    def to_java(raw):
        if re.search('.*_.*', raw):
            return Convert.underscore_to_CamelCase(raw)
        else:
          return raw

    @staticmethod
    def to_constant(raw):
        return Convert.to_c(raw).upper()


def datatypeToString(datatype):
    if datatype == DATATYPE_INT16:
        return "INT16"
    elif datatype == DATATYPE_BOOLEAN:
        return "BOOLEAN"
    elif datatype == DATATYPE_REFRESH_RATE:
        return "REFRESH"
    else:
        return 'Unknown datatype %s' % (datatype)

# i is the number to be transform into byte array, n is the number of bytes to use (little endian)
def toByteString(i, n): 
    return ['(byte)' + str(ord(b)) for b in pack("H", i)][:n]

class WuLink:
    def __init__(self, fromWuObject, fromPropertyId, toWuObject, toPropertyId):
        self.linkSrc = (fromWuObject, fromPropertyId)
        self.linkDst = (toWuObject, toPropertyId)

    def toJava(self):
        print 'wulink toJava'
        return ', '.join(toByteString(int(self.linkSrc[0].getInstanceIndex()), 2) + toByteString(int(self.linkSrc[1]), 1) + toByteString(int(self.linkDst[0].getInstanceIndex()), 2) + toByteString(int(self.linkDst[1]), 1) + toByteString(int(self.linkDst[0].getWuClassId()), 2))
        


class WuType:
    def __init__(self, name, dataType, values=()):
        self._name = name  # WuType's name
        self._dataType = dataType # a string describing data type
        self._allowed_values = values # a tuple of sequential unicode values of the specified type

    def __repr__(self):
        return "WuType %s (type=%s) val:%s" % (self._name, self._dataType, str(self._allowed_values))

    def __contains__(self, value):
        return value in self._allowed_values

    def getName(self):
        return self._name

    def getDataType(self):
        return self._dataType

    def hasAllowedValues(self):
        return self._allowed_values != ()

    def getAllowedValues(self):
        return self._allowed_values

    def getValueInCConstant(self, value):
        return 'WKPF_' + '_'.join([Convert.to_constant(self._dataType), Convert.to_constant(self._name), Convert.to_constant(value)])

    def getValueInJavaConstant(self, value):
        return '_'.join([Convert.to_constant(self._dataType), Convert.to_constant(self._name), Convert.to_constant(value)])

    def getValueInJavaConstByValue(self, value):
        if self.hasAllowedValues():
            return 'GENERATEDWKPF.' + Convert.to_constant(self._dataType) + "_" + Convert.to_constant(self._name) + "_" + Convert.to_constant(value)
        else:
            return value

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

class WuProperty:
    def __init__(self, class_name, name, id, wutype, access, default=None, value=None, status=None):
        self._class_name = class_name
        self._name = name  # an unicode for property's name
        self._id = id      # an integer for property's id
        self._wutype = wutype # a WuType object
        self._access = access
        self._default = default
        self._current_value = value
        self._property_status = status

    def __repr__(self):
        return "WuProperty %s (id=%s, wutype=%s access=%s current_value=%s)" % (self._name, self._id, self._wutype, self._access, str(self._current_value))

    def getName(self):
        return self._name 

    def getWuClassName(self):
        return self._class_name

    def getJavaName(self):
        return Convert.to_constant(self._name)

    def getCConstName(self):
        return "WKPF_" + self.getJavaConstName()

    def getJavaConstName(self):
        return "PROPERTY_" + Convert.to_constant(self._class_name) + "_" + Convert.to_constant(self._name)

    def getPropertyStatus(self):
        return self._property_status

    def setPropertyStatus(self, status):
        self._property_status = status

    def hasDefault(self):
        return self._default != None

    def getDefault(self):
        return self._default

    def setDefault(self, default):
        if self._wutype.hasAllowedValues():
            if default in self._wutype.getAllowedValues():
                self._default = default
        else:
            self._default = default

    def getId(self):
        return self._id

    def setId(self, id):
        self._id = id

    def getWuType(self):
        return self._wutype

    def getDataType(self):
        return self._wutype.getName()

    def setDataType(self, typeName):
        print 'set datatype of name %s of property %s' % (typeName, self.getName())
        if typeName != self._wutype.getName():
            for wutype in all_wutypes:
                if wutype.getName() == typeName:
                    self._wutype = copy.deepcopy(wutype)


    def getAccess(self):
        return self._access

    def getCurrentValue(self):
        return self._current_value

    def setCurrentValue(self, value):
        if self._wutype.hasAllowedValues():
            if value in self._wutype:
                self._current_value = value
        else:
            self._current_value = value

# Now both WuClass and WuObject have node id attribute, because each could represent at different stages of mapping process
class WuClass:
    def __init__(self, name, id, properties, virtual, soft, node_id=None):
        self._name = name  # WuClass's name
        self._id = id      # an integer for class' id
        self._properties = properties  # a dict of WuProperty objects accessed thru the prop's name
        self._virtual = virtual    # a boolean for virtual or native
        self._soft = soft  # a boolean for soft or hard
        self_node_id = node_id

    def __iter__(self):
        for property in self._properties.values():
            yield property

    def __contains__(self, propertyName):
        return propertyName in self._properties

    def __repr__(self):
        return "WuClass %s (id=%d, virt=%s, soft=%s) prop:%s" % (self._name, self._id,str(self._virtual),str(self._soft),str(self._properties))

    def getPropertyByName(self, name):
        return self._properties[name]

    def setPropertyByName(self, name, property):
        self._properties[name] = property

    def getJavaGenClassName(self):
        return "GENERATEDVirtual" + Convert.to_java(self._name) + "WuObject"

    def getJavaClassName(self):
        return "Virtual" + Convert.to_java(self._name) + "WuObject"

    def getCDefineName(self):
        return Convert.to_constant(self.getCName())

    def getCName(self):
        return 'wuclass_' + Convert.to_c(self._name)

    def getCFileName(self):
        return 'GENERATED' + self.getCName()

    def getCUpdateName(self):
        return self.getCName() + "_update"

    def getCPropertyName(self):
        return self.getCName() + "_properties"

    def getCConstName(self):
        return "WKPF_" + self.getJavaConstName()

    def getJavaConstName(self):
        return "WUCLASS_" + Convert.to_constant(self._name)

    def getName(self):
        return self._name

    def getProperties(self):
        return self._properties

    def getId(self):
        return self._id

    def isVirtual(self):
        return self._virtual

    def isSoft(self):
        return self._soft

    def getNodeId(self):
        return self._node_id

    def setNodeId(self, id):
        self._node_id = id

# Now both WuClass and WuObject have node id attribute, because each could represent at different stages of mapping process
# The wuClass in WuObject is just for reference only, the node_id shouldn't be used
class WuObject:
    def __init__(self, wuClass, instanceId, instanceIndex, nodeId=None, portNumber=None, occupied = False, queries=[]):
        self._wuClass = wuClass
        self._instanceId = instanceId
        self._instanceIndex = instanceIndex
        self._nodeId = nodeId
        self._portNumber = portNumber
        self._occupied = occupied
         #queries are [locationQuery, group_sizeQuery], should be replaced by a query class when we have more policy requirements in the future.
        self._queries = queries

    def __repr__(self):
        return 'wuobject(node:'+ str(self._nodeId)+' port:'+ str(self._portNumber)+ ' wuclass id: '+ str(self.getWuClassId())+')'

    def __contains__(self, propertyName):
        return propertyName in self._wuClass.getProperties()

    def __iter__(self):
        for property in self.getProperties().values():
            yield property
    def isOccupied(self):
        return self._occupied
    def setOccupied(self, va = False):
        self._occupied = va

    def addQueries(self, queries):
        for query in queries:
            self.addQuery(query)

    def removeQuery(self, query):
        self._queries.remove(query)

    def addQuery(self, query):
        self._queries.append(query)

    def getQueries(self):
        return self._queries

    def setQueries(self, queries):
        self._queries = queries

    def toJava(self):
        print 'wuobject toJava'
        return ', '.join([str(self.getNodeId()), str(self.getPortNumber())])

    def getWuClass(self):
        return self._wuClass

    def getWuClassName(self):
        return self._wuClass.getName()

    def getWuClassId(self):
        return self._wuClass.getId()

    def getInstanceId(self):
        return self._instanceId

    def getInstanceIndex(self):
        return self._instanceIndex

    def getNodeId(self):
        return self._nodeId

    def setNodeId(self, nodeId):
        self._nodeId = nodeId

    def getPortNumber(self):
        return self._portNumber
    
    def setPortNumber(self, portNumber):
        self._portNumber = portNumber

    def getPropertyByName(self, prop_name):
        return self._wuClass.getPropertyByName(prop_name)

    def getProperties(self):
        return self._wuClass.getProperties()

    def setProperties(self, properties):
        for property in properties:
            self._wuClass.setPropertyByName(property.getName(), property)


class NodeInfo:
    def __init__(self, nodeId, wuClasses=[], wuObjects=[], location=""):
        self.nodeId = nodeId
        self.wuClasses = wuClasses
        self.wuObjects = wuObjects
        self.nativeWuClasses = [wuclass.getId() for wuclass in self.wuClasses if not wuclass.isVirtual()]
        self.virtualWuClasses = [wuclass.getId() for wuclass in self.wuClasses if wuclass.isVirtual()]
# Technically this is not 100% correct since we could make a virtual wuObject even if the native class is present, but this will work for now
        self.nativeWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.getWuClassId() in self.nativeWuClasses]
        self.virtualWuObjects = [wuobject for wuobject in self.wuObjects if wuobject.getWuClassId() in self.virtualWuClasses]
        self.location = location

    def __repr__(self):
        return '(nodeinfo node %d\n\twuclasses %s\n\tnative wuclasses %s\n\tvirtual wuclasses %s\n\twuobjects %s)\n\tlocation %s' % (self.nodeId, str(self.wuClasses), str(self.nativeWuClasses), str(self.virtualWuClasses), str(self.wuObjects), self.location)

    # if both list have stuff
    def isResponding(self):
        return len(self.wuClasses) > 0 or len(self.wuObjects) > 0


# methods for wkpf
def parseXMLString(xml_string, **kwargs):
    dom = xml.dom.minidom.parseString(xml_string)

    wuClasses = {}
    wuTypes = {'short': WuType('short', 'short'), 'boolean': WuType('boolean', 'boolean'), 'refresh_rate': WuType('refresh_rate', 'refresh_rate')}

    classTypes = {'true':True, 'false':False, 'soft':True, 'hard':False}
    accessTypes = {'readwrite':'rw', 'readonly':'ro', 'writeonly':'wo'}

    wuTypedefs_dict = {}

    for wuType in dom.getElementsByTagName('WuTypedef'):
        if wuType.getAttribute('type').lower() == 'enum':
            wuTypes[wuType.getAttribute('name')] = WuType(wuType.getAttribute('name'), wuType.getAttribute('type'), tuple([element.getAttribute('value') for element in wuType.getElementsByTagName('enum')]))
        else:
            wuTypes[wuType.getAttribute('name')] = WuType(wuType.getAttribute('name'), wuType.getAttribute('type'))

    for wuClass in dom.getElementsByTagName('WuClass'):
        wuClassName = wuClass.getAttribute('name')
        wuClassId = int(wuClass.getAttribute('id'),0)
        wuClassProperties = {}
        for i, prop in enumerate(wuClass.getElementsByTagName('property')):
            propType = prop.getAttribute('datatype')
            propName = prop.getAttribute('name')
            if prop.getAttribute('default'):
                defaultValue = prop.getAttribute('default')
            else:
                defaultValue = None

            wuClassProperties[propName] = WuProperty(wuClassName, propName, i, wuTypes[propType], prop.getAttribute('access'), default=defaultValue) 

        wuClasses[wuClassName] = WuClass(wuClassName, wuClassId, wuClassProperties, True if wuClass.getAttribute('virtual').lower() == 'true' else False, True if wuClass.getAttribute('type').lower() == 'soft' else False)

    if 'type' in kwargs and kwargs['type'] == 'wutype':
        return wuTypes

    return wuClasses

def parseXML(xml_path, **kwargs):
    return parseXMLString(open(xml_path).read(), **kwargs)

rootpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
all_wuclasses = parseXML(os.path.join(rootpath, "ComponentDefinitions", "WuKongStandardLibrary.xml")).values()
all_wutypes = parseXML(os.path.join(rootpath, "ComponentDefinitions", "WuKongStandardLibrary.xml"), type='wutype').values()
