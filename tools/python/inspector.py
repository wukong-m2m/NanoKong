#!/usr/bin/python
from optparse import OptionParser
import xml.dom.minidom
import wkpfcomm
import wkpf

## Component XML functions
def getComponentDefinitions(path):
  return xml.dom.minidom.parse(path)

def getComponent(componentId, componentDefinitions):
  for component in componentDefinitions.getElementsByTagName('WuClass'):
    if int(component.getAttribute('id'),0) == componentId:
      return component
  raise Error('No component found with id %d' % (componentId))

def getComponentPropertyCount(componentId, componentDefinitions):
  component = getComponent(componentId, componentDefinitions)
  return len(component.getElementsByTagName('property'))

def getComponentName(componentId, componentDefinitions):
  component = getComponent(componentId, componentDefinitions)
  return component.getAttribute('name')

def getComponentPropertyName(componentId, propertyNumber, componentDefinitions):
  component = getComponent(componentId, componentDefinitions)
  prop = component.getElementsByTagName('property')[propertyNumber]
  return prop.getAttribute('name')

def stringRepresentationIfEnum(componentId, propertyNumber, componentDefinitions, value):
  component = getComponent(componentId, componentDefinitions)
  prop = component.getElementsByTagName('property')[propertyNumber]
  datatype = prop.getAttribute('datatype')
  for typedef in componentDefinitions.getElementsByTagName('WuTypedef'):
    if typedef.getAttribute('name') == datatype:
      enums = typedef.getElementsByTagName('enum')
      return enums[value].getAttribute('value')        
  return str(value)


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
def printNodeInfo(nodeInfo, componentDefinitions):
  print "NodeID: %d" % (nodeInfo.nodeId)
  print "WuClasses:"
  for wuClassInfo in sorted(nodeInfo.wuClasses, key=lambda x:x.wuClassId):
    print "\tClass name %s %s" % (wuClassInfo.name, "VIRTUAL" if wuClassInfo.isVirtual else "NATIVE")
  print "WuObjects:"
  for wuObjectInfo in sorted(nodeInfo.wuObjects, key=lambda x:x.wuClassId):
    print "\tClass name %s" % (wuObjectInfo.wuClassName)
    print "\tPort number %d" % (wuObjectInfo.portNumber)
    print "\tProperties"
    for propertyInfo in wuObjectInfo.properties:
      print "\t\t%d %8s %20s %s" % (propertyInfo.propertyNumber,
                                    wkpf.datatypeToString(propertyInfo.datatype),
                                    propertyInfo.name,
                                    stringRepresentationIfEnum(wuObjectInfo.wuClassId, propertyInfo.propertyNumber, componentDefinitions, propertyInfo.value))

if __name__ == "__main__":
  optionParser = OptionParser("usage: %prog [options]")
  optionParser.add_option("-c", "--component", action="store", type="string", dest="pathComponentXml", help="WuKong Component XML file path")
  (options, args) = optionParser.parse_args()
  if not options.pathComponentXml:
    optionParser.error("invalid component xml, please refer to -h for help")

  componentDefinitions = getComponentDefinitions(options.pathComponentXml)
  nodeIds = wkpfcomm.getNodeIds();
  nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  print nodeInfos
  for nodeInfo in sorted(nodeInfos, key=lambda x:x.nodeId):
    print "============================="
    printNodeInfo(nodeInfo, componentDefinitions)


