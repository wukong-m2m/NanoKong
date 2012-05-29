#!/usr/bin/python
from optparse import OptionParser
import xml.dom.minidom
import wkpfcomm
import wkpf

def find(seq, f):
  """Return first item in sequence where f(item) == True."""
  for item in seq:
    if f(item): 
      return item
  return None

## Component XML functions
def getComponentDefinitions(path):
  return xml.dom.minidom.parse(path)

def getComponent(wuClassId, componentDefinitions):
  for component in componentDefinitions.getElementsByTagName('WuClass'):
    if int(component.getAttribute('id'),0) == wuClassId:
      return component
  raise Error('No component found with id %d' % (wuClassId))

def getComponentPropertyCount(wuClassId, componentDefinitions):
  component = getComponent(wuClassId, componentDefinitions)
  return len(component.getElementsByTagName('property'))

def getComponentName(wuClassId, componentDefinitions):
  component = getComponent(wuClassId, componentDefinitions)
  return component.getAttribute('name')

def getComponentPropertyName(wuClassId, propertyNumber, componentDefinitions):
  component = getComponent(wuClassId, componentDefinitions)
  prop = component.getElementsByTagName('property')[propertyNumber]
  return prop.getAttribute('name')

def stringRepresentationIfEnum(wuClassId, propertyNumber, componentDefinitions, value):
  component = getComponent(wuClassId, componentDefinitions)
  prop = component.getElementsByTagName('property')[propertyNumber]
  datatype = prop.getAttribute('datatype')
  for typedef in componentDefinitions.getElementsByTagName('WuTypedef'):
    if typedef.getAttribute('name') == datatype:
      enums = typedef.getElementsByTagName('enum')
      return enums[value].getAttribute('value')        
  return str(value)

## Flow XML functions
def getFlow(path):
  return xml.dom.minidom.parse(path)

def getRemoteLink(propertyInfo, wuObjectInfo, flowDefinition, mapping, componentDefinitions):
  componentInstanceName = getComponentInstanceName(wuObjectInfo.nodeId, wuObjectInfo.portNumber, mapping)
  propertyName = getComponentPropertyName(wuObjectInfo.wuClassId, propertyInfo.propertyNumber, componentDefinitions)
  for component in flowDefinition.getElementsByTagName('component'):
    for link in component.getElementsByTagName('link'):
      if component.getAttribute('instanceId') == componentInstanceName and link.getAttribute('fromProperty') == propertyName: # Output side of a link
        return ('=>', link.getAttribute('toInstanceId'), link.getAttribute('toProperty'))
      if link.getAttribute('toInstanceId') == componentInstanceName and link.getAttribute('toProperty') == propertyName: # Input side of a link
        return ('<=', component.getAttribute('instanceId'), link.getAttribute('fromProperty'))
  return None

## Mapping functions
def getMapping(path):
  return xml.dom.minidom.parse(path)

def getComponentInstanceName(nodeId, portNumber, mapping):
  entries = mapping.getElementsByTagName('entry')
  for entry in entries:
    if entry.getAttribute('nodeid') == str(nodeId) and entry.getAttribute('portnumber') == str(portNumber):
      return entry.getAttribute('componentname')
  return None # Not mapped to any component in this application

def getNodeAndPortForComponent(componentName, mapping):
  entries = mapping.getElementsByTagName('entry')
  for entry in entries:
    if entry.getAttribute('componentname') == componentName:
      return int(entry.getAttribute('nodeid')), int(entry.getAttribute('portnumber'))
  raise Error('Component not found: %s', (componentName))

## Functions to read data from the nodes
def readPropertyInfo(wuObject, propertyNumber, componentDefinitions):
  class PropertyInfo:
    pass
  propertyInfo = PropertyInfo()
  propertyInfo.name = getComponentPropertyName(wuObject.wuClassId, propertyNumber, componentDefinitions)
  wkpfcommData = wkpfcomm.getProperty(wuObject, propertyNumber)
  print wkpfcommData
  propertyInfo.datatype = wkpfcommData[1]
  propertyInfo.value = wkpfcommData[0]
  propertyInfo.propertyNumber = propertyNumber
  return propertyInfo

def readNodeInfo(nodeId, componentDefinitions):
  wkpfcommNodeInfo = wkpfcomm.getNodeInfo(nodeId)
  for wuClass in wkpfcommNodeInfo.wuClasses:
    wuClass.name = getComponentName(wuClass.wuClassId, componentDefinitions)
  for wuObject in wkpfcommNodeInfo.wuObjects:
    wuObject.wuClassName = getComponentName(wuObject.wuClassId, componentDefinitions)
    wuObject.properties = [readPropertyInfo(wuObject, i, componentDefinitions) for i in range(getComponentPropertyCount(wuObject.wuClassId, componentDefinitions))]
  return wkpfcommNodeInfo

## Print functions
def propertyInfoToString(propertyInfo, wuObjectInfo, componentDefinitions):
  return "%d %8s %15s %10s" % (propertyInfo.propertyNumber,
                             wkpf.datatypeToString(propertyInfo.datatype),
                             propertyInfo.name,
                             stringRepresentationIfEnum(wuObjectInfo.wuClassId, propertyInfo.propertyNumber, componentDefinitions, propertyInfo.value))
  
def printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping):
  for nodeInfo in sorted(nodeInfos, key=lambda x:x.nodeId):
    print "============================="
    print "NodeID: %d" % (nodeInfo.nodeId)
    print "WuClasses:"
    for wuClassInfo in sorted(nodeInfo.wuClasses, key=lambda x:x.wuClassId):
      print "\tClass name %s %s" % (wuClassInfo.name, "VIRTUAL" if wuClassInfo.isVirtual else "NATIVE")
    print "WuObjects:"
    for wuObjectInfo in sorted(nodeInfo.wuObjects, key=lambda x:x.wuClassId):
      if mapping:
        componentInstanceName = getComponentInstanceName(nodeInfo.nodeId, wuObjectInfo.portNumber, mapping)
        if componentInstanceName:
          print "\tComponent instance name: %s" % (componentInstanceName)
      print "\tClass name %s" % (wuObjectInfo.wuClassName)
      print "\tPort number %d" % (wuObjectInfo.portNumber)
      print "\tProperties"
      for propertyInfo in wuObjectInfo.properties:
        remoteLinkAndValue = ""
        if mapping and flowDefinition:
          remoteLink = getRemoteLink(propertyInfo, wuObjectInfo, flowDefinition, mapping, componentDefinitions)
          if remoteLink:
            remoteNodeId, remotePortNumber = getNodeAndPortForComponent(remoteLink[1], mapping)
            remoteNodeInfo = find(nodeInfos, lambda x:x.nodeId == remoteNodeId)
            remoteWuObject = find(remoteNodeInfo.wuObjects, lambda x:x.portNumber == remotePortNumber)
            remotePropertyInfo = find(remoteWuObject.properties, lambda x:x.name == remoteLink[2])
            remoteLinkAndValue = "(%s %s %s = %s)" % (remoteLink[0], remoteLink[1], remoteLink[2], remotePropertyInfo.value)
            if propertyInfo.value != remotePropertyInfo.value:
              remoteLinkAndValue += " !!!!"
        print "\t\t%s %s" % (propertyInfoToString(propertyInfo, wuObjectInfo, componentDefinitions), remoteLinkAndValue)
      print ""

if __name__ == "__main__":
  optionParser = OptionParser("usage: %prog [options]")
  optionParser.add_option("-c", "--component", action="store", type="string", dest="pathComponentXml", help="WuKong Component XML file path")
  optionParser.add_option("-f", "--flow", action="store", type="string", dest="pathFlowXml", help="WuKong Flow XML file path")
  optionParser.add_option("-m", "--mapping", action="store", type="string", dest="pathMappingXml", help="Mapping XML file path")
  (options, args) = optionParser.parse_args()
  if not options.pathComponentXml:
    optionParser.error("invalid component xml, please refer to -h for help")

  componentDefinitions = getComponentDefinitions(options.pathComponentXml)
  flowDefinition = getFlow(options.pathFlowXml) if options.pathFlowXml else None
  mapping = getMapping(options.pathMappingXml) if options.pathMappingXml else None
  nodeIds = wkpfcomm.getNodeIds();
  nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping)


