# vim: ts=2 sw=2
#!/usr/bin/python
from optparse import OptionParser
import xml.dom.minidom
import os
from wkpfcomm import *
import wkpf
import pynvc
import copy

rootpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")

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
  componentInstanceName = getComponentInstanceName(wuObjectInfo.getNodeId(), wuObjectInfo.getPortNumber(), mapping)
  propertyName = propertyInfo.getName()
  #propertyName = getComponentPropertyName(wuObjectInfo.getWuClassId(), propertyInfo.getId(), componentDefinitions)
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
  name = getComponentPropertyName(wuObject.getWuClassId(), propertyNumber, componentDefinitions)
  wkpfcommData = comm.getProperty(wuObject, propertyNumber)
  for candidate in fakedata.all_wuclasses:
    for property in candidate:
      if property.getName() == name:
        ret_prop = copy.deepcopy(property)
        ret_prop.setCurrentValue(wkpfcommData[0])
        ret_prop.setDataType(wkpfcommData[1])
        ret_prop.setPropertyStatus(wkpfcommData[2])
        ret_prop.setId(propertyNumber)


  '''
  propertyInfo.value = wkpfcommData[0]
  propertyInfo.datatype = wkpfcommData[1]
  propertyInfo.status = wkpfcommData[2]
  propertyInfo.propertyNumber = propertyNumber
  '''
  return ret_prop

def readNodeInfo(nodeId, componentDefinitions):
  wkpfcommNodeInfo = comm.getNodeInfo(nodeId)
  if wkpfcommNodeInfo.isResponding():
    '''
    for wuClass in wkpfcommNodeInfo.wuClasses:
      wuClass.name = getComponentName(wuClass.getId(), componentDefinitions)
    '''
    for wuObject in wkpfcommNodeInfo.wuObjects:
      #wuObject.wuClassName = getComponentName(wuObject.getWuClassId(), componentDefinitions)
      wuObject.setProperties([readPropertyInfo(wuObject, i, componentDefinitions) for i in range(getComponentPropertyCount(wuObject.getWuClassId(), componentDefinitions))])
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
      for wuClassInfo in sorted(nodeInfo.wuClasses, key=lambda x:x.getId()):
        print "\tClass name %s %s" % (wuClassInfo.getName(), "VIRTUAL" if wuClassInfo.isVirtual() else "NATIVE")
      print "WuObjects:"
      for wuObjectInfo in sorted(nodeInfo.wuObjects, key=lambda x:x.getWuClassId()):
        if mapping:
          componentInstanceName = getComponentInstanceName(nodeInfo.nodeId, wuObjectInfo.getPortNumber(), mapping)
          if componentInstanceName:
            print "\tComponent instance name: %s" % (componentInstanceName)
        print "\tClass name %s" % (wuObjectInfo.getWuClassName())
        print "\tPort number %d" % (wuObjectInfo.getPortNumber())
        print "\tPropNr  Datatype         Name      value (status) [(remote property value, status)]"
        for propertyInfo in wuObjectInfo:
          propertyInfoString = "\t   %d %8s %15s %10s (%s)" % (propertyInfo.getId(),
                                               #wkpf.datatypeToString(propertyInfo.getDataType()),
                                               propertyInfo.getDataType(),
                                               propertyInfo.getName(),
                                               #stringRepresentationIfEnum(wuObjectInfo.getWuClassId(), propertyInfo.getId(), componentDefinitions, propertyInfo.getCurrentValue()),
                                               propertyInfo.getCurrentValue(),
                                               propertyInfo.getAccess())
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
                  remoteWuObject = find(remoteNodeInfo.wuObjects, lambda x:x.getPortNumber() == remotePortNumber)
                  remotePropertyInfo = find(remoteWuObject.getProperties(), lambda x:x.getName() == remoteLink[2])
                  propertyInfoString += "   (%s %s %s = %s, %0#4x)" % (remoteLink[0], remoteLink[1], remoteLink[2], remotePropertyInfo.getCurrentValue(), remotePropertyInfo.getAccess())
                  if propertyInfo.getCurrentValue() != remotePropertyInfo.getCurrentValue():
                    propertyInfoString += " !!!!"
              except:
                propertyInfoString += " REMOTE PROPERTY NOT FOUND!!!!"
          print propertyInfoString
        print ""

class Inspector:
  def __init__(self, mapping_results):
    self.mapping_results = mapping_results
    self.comm = Communication(0)
    self.node_infos = comm.getAllNodeInfos([wuobject.getNodeId() for wuobject in self.mapping_results.values()])

  def readAllLog(self):
    logs = []
    for node_info in self.node_infos:
      pass

  def readLog(self, node_id):
    logs = []
    if node_id and node_id in [info.nodeId for info in self.node_infos]:
      pass

  def inspectAllProperties():
    properties = []
    for wuobject in self.mapping_results.values():
      properties += wuobject.getProperties()

    for property in properties:
      for node_info in node_infos:
        if node_info.isResponding:
          node_info.wuObjects
        else:
          properties.append()

if __name__ == "__main__":
  optionParser = OptionParser("usage: %prog [options]")
  optionParser.add_option("-c", "--component", action="store", type="string", dest="pathComponentXml", help="WuKong Component XML file path (default NVMROOT/ComponentDefinitions/WuKongStandardLibrary.xml)")
  optionParser.add_option("-m", "--mapping", action="store", type="string", dest="pathMappingXml", help="Mapping XML file path (default NVMROOT/vm/build/avr_mega2560/currentMapping.xml)")
  optionParser.add_option("-f", "--flow", action="store", type="string", dest="pathFlowXml", help="WuKong Flow XML file path (default NVMROOT/vm/build/avr_mega2560/currentFlow.xml)")
  (options, args) = optionParser.parse_args()
  
  if not options.pathComponentXml and os.path.exists(rootpath + "/ComponentDefinitions/WuKongStandardLibrary.xml"):
    options.pathComponentXml = rootpath + "/ComponentDefinitions/WuKongStandardLibrary.xml"
  if not options.pathMappingXml and os.path.exists(rootpath + "/vm/build/avr_mega2560/currentMapping.xml"):
    options.pathMappingXml = rootpath + "/vm/build/avr_mega2560/currentMapping.xml"
  if not options.pathFlowXml and os.path.exists(rootpath + "/vm/build/avr_mega2560/currentFlow.xml"):
    options.pathFlowXml = rootpath + "/vm/build/avr_mega2560/currentFlow.xml"
  if not options.pathComponentXml:
    optionParser.error("invalid component xml, please refer to -h for help")

  comm = getComm()
  componentDefinitions = getComponentDefinitions(options.pathComponentXml)
  mapping = getMapping(options.pathMappingXml) if options.pathMappingXml else None
  flowDefinition = getFlow(options.pathFlowXml) if options.pathFlowXml else None
  nodeIds = comm.getNodeIds();
  nodeInfos = [readNodeInfo(nodeId, componentDefinitions) for nodeId in nodeIds]
  
  print "============================="
  print "Component xml path: %s" % (options.pathComponentXml)
  print "Mapping xml path: %s" % (options.pathMappingXml)
  print "Flow xml path: %s" % (options.pathFlowXml)
  if flowDefinition:
    print "Application in flow xml: %s" % flowDefinition.getElementsByTagName('application')[0].getAttribute('name')
  printNodeInfos(nodeInfos, componentDefinitions, flowDefinition, mapping)


