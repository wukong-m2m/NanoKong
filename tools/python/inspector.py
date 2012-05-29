#!/usr/bin/python
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
def printPropertyInfo(propertyInfo):
  print "\t\t%d %8s %20s %s" % (propertyInfo.propertyNumber, wkpf.datatypeToString(propertyInfo.datatype), propertyInfo.name, propertyInfo.value)

def printWuObjectInfo(wuObjectInfo):
  print "\tClass name %s" % (wuObjectInfo.wuClassName)
  print "\tPort number %d" % (wuObjectInfo.portNumber)
  print "\tProperties"
  for propertyInfo in wuObjectInfo.properties:
    printPropertyInfo(propertyInfo)

def printWuClassInfo(wuClassInfo):
  print "\tClass name %s %s" % (wuClassInfo.name, "VIRTUAL" if wuClassInfo.isVirtual else "NATIVE")

def printNodeInfo(nodeInfo):
  print "NodeID: %d" % (nodeInfo.nodeId)
  print "WuClasses:"
  for wuClassInfo in sorted(nodeInfo.wuClasses, key=lambda x:x.wuClassId):
    printWuClassInfo(wuClassInfo)
  print "WuObjects:"
  for wuObjectInfo in sorted(nodeInfo.wuObjects, key=lambda x:x.wuClassId):
    printWuObjectInfo(wuObjectInfo)

if __name__ == "__main__":
  componentDefinitions = getComponentDefinitions('../../ComponentDefinitions/WuKongStandardLibrary.xml')
  nodeIds = wkpfcomm.getNodeIds();
  nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  print nodeInfos
  for nodeInfo in sorted(nodeInfos, key=lambda x:x.nodeId):
    print "============================="
    printNodeInfo(nodeInfo)
