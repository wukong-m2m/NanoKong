# vim: ts=2 sw=2

import sys, os, traceback
sys.path.append(os.path.abspath("../xml2java"))
sys.path.append(os.path.abspath("../../master"))
from wkpf import *
from locationTree import *
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
import logging
import logging.handlers
import wukonghandler
import copy
from locationParser import *
from wkpfcomm import *
from codegen import generateCode
from xml2java.translator import generateJava
import copy
from threading import Thread
import traceback
import time
import re
import StringIO
import shutil, errno
import datetime
from subprocess import Popen, PIPE, STDOUT

from configuration import *
from globals import *

OK = 0
NOTOK = 1

def constructHeartbeatGroups(heartbeatGroups, routingTable, allCandidateIds):
  groups = 0
  del heartbeatGroups[:]
  heartbeatGroups.append({'period': 0, 'members': []})
  while len(allCandidateIds) > 0:
    if len(heartbeatGroups[groups]['members']) == 0:
      heartbeatGroups[groups]['members'].append(allCandidateIds.pop(0))
    elif len(heartbeatGroups[groups]['members']) == 1:
      pivot = heartbeatGroups[groups]['members'][0]
      if pivot in routingTable:
        for neighbor in routingTable[pivot]:
          if neighbor in allCandidateIds:
            heartbeatGroups[groups]['members'].append(neighbor)
            allCandidateIds.remove(neighbor)
      if len(allCandidateIds) > 0:
        groups += 1 #next group
        heartbeatGroups.insert(groups, {'period': 0, 'members': []})

# assign periods
# FTComponentPolicy = {wuclass id: {'level': 3, 'reaction': 4}}
def determinePeriodForHeartbeatGroups(wuObjects, heartbeatGroups, FTComponentPolicy):
  logging.info('FTComponentPolicy')
  logging.info(FTComponentPolicy)
  for group in heartbeatGroups:
    period = None
    for nodeId in group['members']:
      for candidates in wuObjects.values():
        for wuobject in candidates:
          if nodeId == wuobject.getNodeId():
            if period == None or period > FTComponentPolicy[str(wuobject.getWuClassId())]['reaction']:
              period = FTComponentPolicy[str(wuobject.getWuClassId())]['reaction']
    group['period'] = period

def sortCandidates(wuObjects):
    nodeScores = {}
    for candidates in wuObjects:
      for node in candidates:
        if node[0] in nodeScores:
          nodeScores[node[0]] += 1
        else:
          nodeScores[node[0]] = 1

    for candidates in wuObjects:
      sorted(candidates, key=lambda node: nodeScores[node[0]])

def firstCandidate(app, FTComponentPolicy, heartbeatGroups, routingTable, wuObjects, locTree):
    #input: nodes, WuObjects, WuLinks, WuClassDefsm, wuObjects is a list of wuobject list corresponding to group mapping
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes

    for i, wuObject in enumerate(wuObjects.values()):
        candidateSet = set()
        queries = wuObject[0].getQueries()

        # filter by query
        if queries == []:
            locParser = LocationParser(locTree)
            candidateSet = locTree.getAllAliveNodeIds()
        else:
            locParser = LocationParser(locTree) # get the location query for a component, TODO:should consider other queries too later
            try:
                print "query:",queries[0]
                tmpSet = locParser.parse(queries[0])
                print "parsing result:", tmpSet
            except:
                exc_type, exc_value, exc_traceback = sys.exc_info()
                set_wukong_status(str(exc_type)+str(exc_value))
                return False
                
            print "parsing result:", tmpSet
            logging.info("query")
            logging.info(queries[0])

            logging.info("location Tree")
            logging.info(locTree.printTree())

            if len(tmpSet) > 0:
                candidateSet = tmpSet
            else:
                app.error('Locality conditions for component wuclass id "%s" are too strict; no available candidate found' % (wuObject[0].getWuClass().getId()))
                return False

        #actualGroupSize = queries[1] #queries[1] is the suggested group size
        actualGroupSize = FTComponentPolicy[str(wuObject[0].getWuClassId())]['level'] #the suggested group size

        # filter by available wuclasses for non-virtual components
        node_infos = [locTree.getNodeInfoById(nodeId) for nodeId in candidateSet]

        candidateSet = [] # a list of [sensor id, port no., has native wuclass] pairs
        logging.info(wuObject[0].getWuClass().getId())
        for node_info in node_infos:
            logging.info("node id %d" % (node_info.nodeId))
            if wuObject[0].getWuClass().getId() in [wuclass.getId() for wuclass in node_info.wuClasses]: #native class, use the wuobj port
                found = False
                for wuobject in node_info.wuObjects:
                    if wuobject.getWuClassId()== wuObject[0].getWuClassId() and wuobject.isOccupied() == False:
                        logging.info('using existing wuobject')
                        logging.info(wuobject)
                        portNumber = wuobject.getPortNumber() 
                        candidateSet.append([node_info.nodeId, portNumber, True])
                        found = True
                if not found:
                    logging.info('creating wuobject')
                    sensorNode = locTree.sensor_dict[node_info.nodeId]
                    sensorNode.initPortList(forceInit = False)
                    portNo = sensorNode.reserveNextPort() 
                    logging.info('at port %d', portNo)
                    candidateSet.append([node_info.nodeId, portNo, True])
            elif wuObject[0].getWuClass().isVirtual(): #virtual wuclass, create new port number
                logging.info('creating virtual wuobject')
                sensorNode = locTree.sensor_dict[node_info.nodeId]
                sensorNode.initPortList(forceInit = False)
                portNo = sensorNode.reserveNextPort() 
                if portNo != None:  #means Not all ports in node occupied, still can assign new port
                    candidateSet.append([node_info.nodeId, portNo, False])
                
        
        if len(candidateSet) == 0:
          app.error ('[ERROR] No node could be mapped for component id '+str(wuObject[0].getWuClassId()))
          return False

        #temporary hack
        #candidateSet = sorted(candidateSet, key=lambda candidate: candidate[2])
        #candidateSet.reverse()

        app.warning('unsorted candidates are ' + str(candidateSet))

        #integrity check
        if len(candidateSet) < actualGroupSize:
          app.warning('[WARNING] candidates for component do not satisfy the FT policy')
        else:
          app.info('Minimum redundancy level for component ' + str(wuObject) + ' is ' + str(actualGroupSize) + ' is satisfied')

        shadow = copy.deepcopy(wuObject[0])
        del wuObject[:]
        
        # minimum group size will not affect the result
        for candidate in candidateSet:
          tmp = copy.deepcopy(shadow)
          tmp.setNodeId(candidate[0])
          tmp.setPortNumber(candidate[1])
          tmp.setHasWuClass(candidate[2])
          tmp.setOccupied(True)
          wuObject.append(tmp)

        for unused in candidateSet[actualGroupSize:]:
          senNd = locTree.getSensorById(unused[0])
          if unused[1] in senNd.temp_port_list:          #not in means it is a native wuclass port, so no need to roll back	
            senNd.temp_port_list.remove(unused[1])
            senNd.port_list.remove(unused[1])

        logging.info(wuObject)

    # construct heartbeat groups plus period assignment
    allCandidateIds = []
    for candidates in wuObjects.values():
      allCandidateIds += candidates
    allCandidateIds = [node.getNodeId() for node in allCandidateIds]
    nodeInfos = [locTree.getNodeInfoById(nodeId) for nodeId in allCandidateIds]
    constructHeartbeatGroups(heartbeatGroups, routingTable, allCandidateIds)
    determinePeriodForHeartbeatGroups(wuObjects, heartbeatGroups, FTComponentPolicy)
    logging.info('heartbeatGroups constructed, periods assigned')
    logging.info(heartbeatGroups)

    #sort candidates
    sortCandidates(wuObjects)

    #delete and roll back all reservation during mapping after mapping is done, next mapping will overwritten the current one
    for key in wuObjects.keys():
        comp = wuObjects[key]
        for wuobj in comp:
            senNd = locTree.getSensorById(wuobj.getNodeId())
            for j in senNd.temp_port_list:
                senNd.port_list.remove(j)
            senNd.temp_port_list = []
    return True

class WuApplication:
  def __init__(self, id='', name='', desc='', file='', dir='', outputDir="", templateDir=TEMPLATE_DIR, componentXml=open(COMPONENTXML_PATH).read()):
    self.id = id
    self.name = name
    self.desc = desc
    self.file = file
    self.xml = ''
    self.dir = dir
    self.compiler = None
    self.version = 0
    self.returnCode = NOTOK
    self.status = ""
    self.deployed = False
    self.mapping_results = {}
    self.mapper = None
    self.inspector = None
    # 5 levels: self.logger.debug, self.logger.info, self.logger.warn, self.logger.error, self.logger.critical
    self.logger = logging.getLogger('wukong')
    self.logger.setLevel(logging.DEBUG) # to see all levels
    self.loggerHandler = wukonghandler.WukongHandler(1024 * 3, target=logging.FileHandler(os.path.join(self.dir, 'compile.log')))
    self.logger.addHandler(self.loggerHandler)

    # For Mapper
    self.applicationDom = ""
    self.applicationName = ""
    self.destinationDir = outputDir
    self.templateDir = templateDir
    self.componentXml = componentXml

    self.wuClasses = {}
    self.wuObjects = {}
    self.wuLinks = []
    self.heartbeatGroups = []
    self.FTComponentPolicy = {}

  def setFlowDom(self, flowDom):
    self.applicationDom = flowDom
    self.applicationName = flowDom.getElementsByTagName('application')[0].getAttribute('name')

  def setOutputDir(self, outputDir):
    self.destinationDir = outputDir

  def setTemplateDir(self, templateDir):
    self.templateDir = templateDir

  def setComponentXml(self, componentXml):
    self.componentXml = componentXml

  def logs(self):
    self.loggerHandler.retrieve()
    logs = open(os.path.join(self.dir, 'compile.log')).readlines()
    return logs

  def retrieve(self):
    return self.loggerHandler.retrieve()

  def info(self, line):
    self.logger.info(line)
    self.version += 1

  def error(self, line):
    self.logger.error(line)
    self.version += 2

  def warning(self, line):
    self.logger.warning(line)
    self.version += 1

  def updateXML(self, xml):
    self.xml = xml
    self.setFlowDom(parseString(self.xml))
    self.saveConfig()
    f = open(os.path.join(self.dir, self.id + '.xml'), 'w')
    f.write(xml)
    f.close()

  def loadConfig(self):
    config = json.load(open(os.path.join(self.dir, 'config.json')))
    self.id = config['id']
    self.name = config['name']
    self.desc = config['desc']
    self.dir = config['dir']
    self.xml = config['xml']
    try:
      dom = parseString(self.xml)
      self.setFlowDom(dom)
    except ExpatError:
      pass

  def saveConfig(self):
    json.dump(self.config(), open(os.path.join(self.dir, 'config.json'), 'w'))

  def getReturnCode(self):
    return self.returnCode

  def getStatus(self):
    return self.status

  def config(self):
    return {'id': self.id, 'name': self.name, 'desc': self.desc, 'dir': self.dir, 'xml': self.xml, 'version': self.version}

  def __repr__(self):
    return json.dumps(self.config())

  def parseComponents(self):
      self.wuClasses = parseXMLString(self.componentXml) # an array of wuClasses

  def parseApplicationXML(self):
      self.wuObjects = {}
      self.wuLinks = []
      instanceIdSet = []
      # TODO: parse application XML to generate WuClasses, WuObjects and WuLinks
      for index, componentTag in enumerate(self.applicationDom.getElementsByTagName('component')):
          # make sure application component is found in wuClassDef component list
          assert componentTag.getAttribute('type') in self.wuClasses.keys()
          instanceId=componentTag.getAttribute('instanceId')
          wuClass = None
          if instanceId in instanceIdSet:
              wuClass = self.wuObjects[instanceId]
          else:
              instanceIdSet.append(instanceId)
              # a copy of wuclass
              wuClass = copy.deepcopy(self.wuClasses[componentTag.getAttribute('type')])

          # set properties from FBP to wuclass properties
          for propertyTag in componentTag.getElementsByTagName('signalProperty'):
              for attr in propertyTag.attributes.values():
                  if len(attr.value) !=0:
                      wuClass.setPropertyValueByName(attr.name, attr.value)

          for propertyTag in componentTag.getElementsByTagName('actionProperty'):
              for attr in propertyTag.attributes.values():
                  if len(attr.value) !=0:
                      wuClass.setPropertyValueByName(attr.name, attr.value)
                      
          queries = []
          #assume there is one location requirement per component in application.xml
          for locationQuery in componentTag.getElementsByTagName('location'):
              queries.append(locationQuery.getAttribute('requirement'))
          if len(queries) ==0:
              queries.append ('')
          elif len (queries) > 1:
              logging.error('input file violating the assumption there is one location requirement per component in application.xml')
          # nodeId is not used here, portNumber is generated later
          wuObj = WuObject(wuClass=wuClass, instanceId=componentTag.getAttribute('instanceId'), instanceIndex=index, queries=queries)

          #TODO: for each component, there is a list of wuObjs (length depending on group_size)
          # Instance id is only for mapping temporarily (since there could be
          # duplicate wuobjects)
          self.wuObjects[wuObj.getInstanceId()] = [wuObj]

          self.FTComponentPolicy[str(wuObj.getWuClassId())] = {'level': None,
            'reaction': None}

          #FTComponentPolicy
          #assume there is one group_size requirement per component
          for groupSizeQuery in componentTag.getElementsByTagName('group_size'):
              self.FTComponentPolicy[str(wuObj.getWuClassId())]['level'] = int(groupSizeQuery.getAttribute('requirement'))
          #assume there is one reaction_time requirement per component
          for reactionTimeQuery in componentTag.getElementsByTagName('reaction_time'):
              self.FTComponentPolicy[str(wuObj.getWuClassId())]['reaction'] = int(reactionTimeQuery.getAttribute('requirement'))

      #assumption: at most 99 properties for each instance, at most 999 instances
      linkSet = []  #store hashed result of links to avoid duplicated links: (fromInstanceId*100+fromProperty)*100000+toInstanceId*100+toProperty
      # links
      for linkTag in self.applicationDom.getElementsByTagName('link'):
          fromInstanceId = linkTag.parentNode.getAttribute('instanceId')
          fromWuObject = self.wuObjects[fromInstanceId][0]
          fromPropertyId = fromWuObject.getPropertyByName(linkTag.getAttribute('fromProperty')).getId()
          
          toInstanceId = linkTag.getAttribute('toInstanceId')
          toWuObject = self.wuObjects[toInstanceId][0]
          toPropertyId = toWuObject.getPropertyByName(linkTag.getAttribute('toProperty')).getId()
          hash_value = (int(fromInstanceId)*100+int(fromPropertyId))*100000+int(toInstanceId)*100+int(toPropertyId)
          if hash_value not in linkSet:
              linkSet.append(hash_value)
              self.wuLinks.append( WuLink(fromWuObject, fromPropertyId, toWuObject, toPropertyId) )

  def mapping(self, locTree, routingTable, mapFunc=firstCandidate):
      #input: nodes, WuObjects, WuLinks, WuClassDefs
      #output: assign node id to WuObjects
      # TODO: mapping results for generating the appropriate instiantiation for different nodes
      
      return mapFunc(self, self.FTComponentPolicy, self.heartbeatGroups, routingTable, self.wuObjects, locTree)

  def map(self, location_tree, routingTable):
    self.parseComponents()
    self.parseApplicationXML()
    self.mapping(location_tree, routingTable)
    self.mapping_results = self.wuObjects
    logging.info("Mapping results")
    logging.info(self.mapping_results)

  def deploy_with_discovery(self,*args):
    node_ids = [info.nodeId for info in getComm().getActiveNodeInfos(force=True)]
    self.deploy(node_ids,*args)

  def deploy(self, destination_ids, platforms):
    master_busy()
    app_path = self.dir
    
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)

      self.status = "Generating java library code"
      gevent.sleep(0)

      # CodeGen
      self.info('==Generating necessary files for wukong')
      try:
        generateCode(self)
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.error(e)
        return False

      self.status = "Generating java application"
      gevent.sleep(0)

      # Mapper results, already did in map_application
      # Generate java code
      self.info('==Generating application code in target language (Java)')
      try:
        generateJava(self)
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.error(e)
        self.status = "Error generating java application"
        gevent.sleep(0)
        return False

      self.status = "Compressing java to bytecode format"
      gevent.sleep(0)

      # Generate nvmdefault.h
      self.info('==Compressing application code to bytecode format')
      pp = Popen('cd %s; make application FLOWXML=%s' % (platform_dir, self.id), shell=True, stdout=PIPE, stderr=PIPE)
      self.returnCode = None
      (infomsg,errmsg) = pp.communicate()

      self.info(infomsg)

      self.error(errmsg)
      self.version += 1
      if pp.returncode != 0:
        self.error('==Error generating nvmdefault.h')
        self.status = "Error generating nvmdefault.h"
        gevent.sleep(0)
        return False
      self.info('==Finishing compression')
      if  SIMULATION !=0:
          #we don't do any real deployment for simulation, 
          return False

      self.status = "Deploying bytecode to nodes"
      gevent.sleep(0)

      comm = getComm()
      # Deploy nvmdefault.h to nodes
      self.info('==Deploying to nodes %s' % (str(destination_ids)))
      remaining_ids = copy.deepcopy(destination_ids)

      for node_id in destination_ids:
        remaining_ids.remove(node_id)
        self.status = "Deploying bytecode to node %d, remaining %s" % (node_id, str(remaining_ids))
        gevent.sleep(0)
        self.info('==Deploying to node id: %d' % (node_id))
        ret = False
        retries = 3
        while retries > 0:
          if not comm.reprogram(node_id, os.path.join(platform_dir, 'nvmdefault.h'), retry=False):
            self.status = "Deploying unsucessful for node %d, trying again" % (node_id)
            gevent.sleep(0)
            self.error('==Node not deployed successfully, retries = %d' % (retries))
            retries -= 1
          else:
            ret = True
            break
        if not ret:
          self.status = "Deploying unsuccessful"
          gevent.sleep(0)
          return False
        '''
        pp = Popen('cd %s; make nvmcomm_reprogram NODE_ID=%d FLOWXML=%s' % (platform_dir, node_id, app.id), shell=True, stdout=PIPE, stderr=PIPE)
        app.returnCode = None
        while pp.poll() == None:
          print 'polling from popen...'
          line = pp.stdout.readline()
          if line != '':
            app.info(line)

          line = pp.stderr.readline()
          if line != '':
            app.error(line)
          app.version += 1
        app.returnCode = pp.returncode
        '''
        self.info('==Deploying to node completed')
    self.info('==Deployment has completed')
    self.status = "Deploying sucess"
    self.status = ""
    gevent.sleep(0)
    master_available()
    return True

  def reconfiguration(self):
    global location_tree
    global routingTable
    master_busy()
    self.status = "Start reconfiguration"
    node_infos = getComm().getActiveNodeInfos(force=True)
    location_tree = LocationTree(LOCATION_ROOT)
    location_tree.buildTree(node_infos)
    routingTable = getComm().getRoutingInformation()
    self.map(location_tree, routingTable)
    self.deploy([info.nodeId for info in node_infos], DEPLOY_PLATFORMS)
    master_available()
