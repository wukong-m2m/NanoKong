#!/usr/bin/python
from optparse import OptionParser
import xml.dom.minidom
import os
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

def getRemoteLinks(propertyInfo, wuObjectInfo, flowDefinition, mapping, componentDefinitions):
  componentInstanceName = getComponentInstanceName(wuObjectInfo.nodeId, wuObjectInfo.portNumber, mapping)
  propertyName = getComponentPropertyName(wuObjectInfo.wuClassId, propertyInfo.propertyNumber, componentDefinitions)
  links = []
  for component in flowDefinition.getElementsByTagName('component'):
    for link in component.getElementsByTagName('link'):
      if component.getAttribute('instanceId') == componentInstanceName and link.getAttribute('fromProperty') == propertyName: # Output side of a link
        links.append(('=>', link.getAttribute('toInstanceId'), link.getAttribute('toProperty')))
      if link.getAttribute('toInstanceId') == componentInstanceName and link.getAttribute('toProperty') == propertyName: # Input side of a link
        links.append(('<=', component.getAttribute('instanceId'), link.getAttribute('fromProperty')))
  return links

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
  propertyInfo.value = wkpfcommData[0]
  propertyInfo.datatype = wkpfcommData[1]
  propertyInfo.status = wkpfcommData[2]
  propertyInfo.propertyNumber = propertyNumber
  return propertyInfo

def readNodeInfo(nodeId, componentDefinitions):
  wkpfcommNodeInfo = wkpfcomm.getNodeInfo(nodeId)
  if wkpfcommNodeInfo.isResponding:
    for wuClass in wkpfcommNodeInfo.wuClasses:
      wuClass.name = getComponentName(wuClass.wuClassId, componentDefinitions)
    for wuObject in wkpfcommNodeInfo.wuObjects:
      wuObject.wuClassName = getComponentName(wuObject.wuClassId, componentDefinitions)
      wuObject.properties = [readPropertyInfo(wuObject, i, componentDefinitions) for i in range(getComponentPropertyCount(wuObject.wuClassId, componentDefinitions))]
  return wkpfcommNodeInfo

## Print functions
def printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping):
  for nodeInfo in sorted(nodeInfos, key=lambda x:x.nodeId):
    print "============================="
    print "NodeID: %d" % (nodeInfo.nodeId)
    if not nodeInfo.isResponding:
      print "NOT RESPONDING"
    else:
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
        print "\tPropNr  Datatype         Name      value (status) [(remote property value, status)]"
        for propertyInfo in wuObjectInfo.properties:
          propertyInfoString = "\t   %d %8s %15s %10s (%0#4x)" % (propertyInfo.propertyNumber,
                                               wkpf.datatypeToString(propertyInfo.datatype),
                                               propertyInfo.name,
                                               stringRepresentationIfEnum(wuObjectInfo.wuClassId, propertyInfo.propertyNumber, componentDefinitions, propertyInfo.value),
                                               propertyInfo.status)
          remoteLinkAndValue = ""
          if mapping and flowDefinition:
            remoteLinks = getRemoteLinks(propertyInfo, wuObjectInfo, flowDefinition, mapping, componentDefinitions)
            for remoteLink in remoteLinks:
              try:
                remoteNodeId, remotePortNumber = getNodeAndPortForComponent(remoteLink[1], mapping)
                remoteNodeInfo = find(nodeInfos, lambda x:x.nodeId == remoteNodeId)
                if remoteNodeInfo == None:
                  propertyInfoString += " NODE NOT FOUND"
                elif not remoteNodeInfo.isResponding:
                  propertyInfoString += " NODE %d NOT RESPONDING" % (remoteNodeInfo.nodeId)                
                else:
                  remoteWuObject = find(remoteNodeInfo.wuObjects, lambda x:x.portNumber == remotePortNumber)
                  remotePropertyInfo = find(remoteWuObject.properties, lambda x:x.name == remoteLink[2])
                  propertyInfoString += "   (%s %s %s = %s, %0#4x)" % (remoteLink[0], remoteLink[1], remoteLink[2], remotePropertyInfo.value, remotePropertyInfo.status)
                  if propertyInfo.value != remotePropertyInfo.value:
                    propertyInfoString += " !!!!"
              except:
                propertyInfoString += " REMOTE PROPERTY NOT FOUND!!!!"
          print propertyInfoString
        print ""

if __name__ == "__main__":
  optionParser = OptionParser("usage: %prog [options]")
  optionParser.add_option("-c", "--component", action="store", type="string", dest="pathComponentXml", help="WuKong Component XML file path (default NVMROOT/ComponentDefinitions/WuKongStandardLibrary.xml)")
  optionParser.add_option("-m", "--mapping", action="store", type="string", dest="pathMappingXml", help="Mapping XML file path (default NVMROOT/vm/build/avr_mega2560/currentMapping.xml)")
  optionParser.add_option("-f", "--flow", action="store", type="string", dest="pathFlowXml", help="WuKong Flow XML file path (default NVMROOT/vm/build/avr_mega2560/currentFlow.xml)")
  (options, args) = optionParser.parse_args()
  
  rootpath = os.path.dirname(os.path.abspath(__file__)) + "/../.."
  if not options.pathComponentXml and os.path.exists(rootpath + "/ComponentDefinitions/WuKongStandardLibrary.xml"):
    options.pathComponentXml = rootpath + "/ComponentDefinitions/WuKongStandardLibrary.xml"
  if not options.pathMappingXml and os.path.exists(rootpath + "/vm/build/avr_mega2560/currentMapping.xml"):
    options.pathMappingXml = rootpath + "/vm/build/avr_mega2560/currentMapping.xml"
  if not options.pathFlowXml and os.path.exists(rootpath + "/vm/build/avr_mega2560/currentFlow.xml"):
    options.pathFlowXml = rootpath + "/vm/build/avr_mega2560/currentFlow.xml"
  if not options.pathComponentXml:
    optionParser.error("invalid component xml, please refer to -h for help")

  componentDefinitions = getComponentDefinitions(options.pathComponentXml)
  mapping = getMapping(options.pathMappingXml) if options.pathMappingXml else None
  flowDefinition = getFlow(options.pathFlowXml) if options.pathFlowXml else None
  nodeIds = wkpfcomm.getNodeIds();
  nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  
  print "============================="
  print "Component xml path: %s" % (options.pathComponentXml)
  print "Mapping xml path: %s" % (options.pathMappingXml)
  print "Flow xml path: %s" % (options.pathFlowXml)
  if flowDefinition:
    print "Application in flow xml: %s" % flowDefinition.getElementsByTagName('application')[0].getAttribute('name')
  printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping)


