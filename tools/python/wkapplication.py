# vim: ts=2 sw=2

import sys, os, traceback
sys.path.append(os.path.abspath("../xml2java"))
sys.path.append(os.path.abspath("../../master"))
from wkpf import *
from locationTree import *
from xml.dom.minidom import parse, parseString
import simplejson as json
import logging
import logging.handlers
import wukonghandler
import copy
from URLParser import *
from wkpfcomm import *
from codegen import generateCode
from translator import generateJava
import copy

from configuration import *

OK = 0
NOTOK = 1

def firstCandidate(app, wuObjects, locTree):
    #input: nodes, WuObjects, WuLinks, WuClassDefsm, wuObjects is a list of wuobject list corresponding to group mapping
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes

    for i, wuObject in enumerate(wuObjects.values()):
        candidateSet = set()
        queries = wuObject[0].getQueries()

        # filter by query
        if queries == []:
            locURLHandler = LocationURL(None, locTree)
            candidateSet = locTree.root.getAllNodes()
        else:
            locURLHandler = LocationURL(queries[0], locTree) # get the location query for a component, TODO:should consider other queries too later

            '''
            try:
              locURLHandler.parseURL()
              tmpSet = locURLHandler.solveParseTree(locURLHandler.parseTreeRoot)
              if len(tmpSet) > 0:
                  candidateSet = tmpSet
              else:
                  app.error('Conditions for component ', str(len(candidateSet)), '(start form 0) too strict, no available candidate found')
                  return False
            except Exception as e:
              app.error(e)
              return False
            '''

            locURLHandler.parseURL()
            tmpSet = locURLHandler.solveParseTree()
            if len(tmpSet) > 0:
                candidateSet = tmpSet
            else:
                app.error('Locality conditions for component "%s" are too strict; no available candidate found' % (wuObject.getInstanceId()))
                return False

        # filter by available wuclasses for non-virtual components
        node_infos = [locTree.getNodeInfoById(nodeId) for nodeId in candidateSet]
        candidateSet = [] # a list of [sensor id, port no.] pairs
        for node_info in node_infos:
            if wuObject[0].getWuClass().getId() in node_info.nativeWuClasses: #native class, use the wuobj port
                for wuobject in node_info.wuObjects:
                    if wuobject.getWuClassId()== wuObject[0].getWuClassId() and wuobject.isOccupied() == False:
                        portNumber = wuobject.getPortNumber() 
                        candidateSet.append([node_info.nodeId, portNumber])
                        break
            elif wuObject[0].getWuClass().isVirtual(): #virtual wuclass, create new port number
                sensorNode = locTree.sensor_dict[node_info.nodeId]
                sensorNode.initPortList(forceInit = False)
                portNo = sensorNode.reserveNextPort() 
                if portNo != None:  #means Not all ports in node occupied, still can assign new port
                    candidateSet.append([node_info.nodeId, portNo])
                
        
        if len(candidateSet) == 0:
          app.error ('No node could be mapped for component'+str(wuObject[0].getInstanceId()))
          return False
        actualGroupSize = queries[1] #queries[1] is the suggested group size
        if actualGroupSize > len(candidateSet):
            actualGroupSize = len(candidateSet)
        groupMemberIds = candidateSet[:actualGroupSize]
        #select the first candidates who satisfies the condiditon
        app.warning('will select the first '+ str(actualGroupSize)+' in this candidateSet ' + str(candidateSet))
        wuObject[0].setNodeId(candidateSet[0][0])    
        wuObject[0].setPortNumber(candidateSet[0][1])
        for i in range(1, actualGroupSize):
            tmp = copy.deepcopy(wuObject[0])
            tmp.setNodeId(candidateSet[i][0])    
            tmp.setPortNumber(candidateSet[i][1])
            tmp.setOccupied(True)
            wuObject.append(tmp)
            
        
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
    self.status = "Idle"
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
    print 'info log'
    self.logger.info(line)
    self.version += 1

  def error(self, line):
    print 'error log'
    self.logger.error(line)
    self.version += 2

  def warning(self, line):
    print 'warning log'
    self.logger.warning(line)
    self.version += 1

  def updateXML(self, xml):
    print 'updateConfig'
    self.xml = xml
    self.setFlowDom(parseString(self.xml))
    self.saveConfig()
    f = open(os.path.join(self.dir, self.id + '.xml'), 'w')
    f.write(xml)
    f.close()

  def loadConfig(self):
    print 'loadConfig'
    config = json.load(open(os.path.join(self.dir, 'config.json')))
    self.id = config['id']
    self.name = config['name']
    self.desc = config['desc']
    self.dir = config['dir']
    self.xml = config['xml']
    self.setFlowDom(parseString(self.xml))

  def saveConfig(self):
    print 'saveConfig'
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

      # TODO: parse application XML to generate WuClasses, WuObjects and WuLinks
      for index, componentTag in enumerate(self.applicationDom.getElementsByTagName('component')):
          # make sure application component is found in wuClassDef component list
          assert componentTag.getAttribute('type') in self.wuClasses.keys()

          # a copy of wuclass
          wuClass = copy.deepcopy(self.wuClasses[componentTag.getAttribute('type')])

          # TODO: for java variable instantiation
          for propertyTag in componentTag.getElementsByTagName('property'):
              assert propertyTag.getAttribute('name') in wuClass

              wuProperty = wuClass.getPropertyByName(propertyTag.getAttribute('name'))
              if propertyTag.getAttribute('default'):
                  wuProperty.setDefault(propertyTag.getAttribute('default'))

          queries = []
          #assume there is one location requirement per component in application.xml
          for locationQuery in componentTag.getElementsByTagName('location'):
              queries.append(locationQuery.getAttribute('requirement'))
          if len(queries) ==0:
              queries.append ('')
          elif len (queries) > 1:
              logging.error('input file violating the assumption there is one location requirement per component in application.xml')
          #assume there is one group_size requirement per component in application.xml
          for groupSizeQuery in componentTag.getElementsByTagName('group_size'):
              queries.append(eval(groupSizeQuery.getAttribute('requirement')))
          if len(queries) ==1:
              queries.append (1)
          elif len (queries) > 2:
              logging.error('input file violating the assumption there is one group_size requirement per component in application.xml')
          # nodeId is not used here, portNumber is generated later
          wuObj = WuObject(wuClass=wuClass, instanceId=componentTag.getAttribute('instanceId'), instanceIndex=index, queries=queries)

          #for each component, there is a list of wuObjs (length depending on group_size)
          self.wuObjects[wuObj.getInstanceId()] = [wuObj]

      # links
      for linkTag in self.applicationDom.getElementsByTagName('link'):
          fromWuObject = self.wuObjects[linkTag.parentNode.getAttribute('instanceId')][0]
          fromPropertyId = fromWuObject.getPropertyByName(linkTag.getAttribute('fromProperty')).getId()

          toWuObject = self.wuObjects[linkTag.getAttribute('toInstanceId')][0]
          toPropertyId = toWuObject.getPropertyByName(linkTag.getAttribute('toProperty')).getId()

          self.wuLinks.append( WuLink(fromWuObject, fromPropertyId, toWuObject, toPropertyId) )

  def map(self, location_tree):
    self.parseComponents()
    self.parseApplicationXML()
    self.mapping(location_tree)
    self.mapping_results = self.wuObjects
    print "Mapping result"+str(self.mapping_results)

  def mapping(self, locTree, mapFunc=firstCandidate):
      #input: nodes, WuObjects, WuLinks, WuClassDefs
      #output: assign node id to WuObjects
      # TODO: mapping results for generating the appropriate instiantiation for different nodes
      
      return mapFunc(self, self.wuObjects, locTree)

  def deploy(self, destination_ids, platforms):
    
    app_path = self.dir
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)

      # CodeGen
      self.info('Generating necessary files for wukong')
      try:
        generateCode(self)
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.error(e)
        return False

      # Mapper results, already did in map_application
      # Generate java code
      self.info('Generating application code in target language (Java)')
      try:
        generateJava(self)
      except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_exception(exc_type, exc_value, exc_traceback,
                                      limit=2, file=sys.stdout)
        self.error(e)
        return False

      # Generate nvmdefault.h
      self.info('Compressing application code to bytecode format')
      print 'changing to path: %s...' % platform_dir
      pp = Popen('cd %s; make application FLOWXML=%s' % (platform_dir, self.id), shell=True, stdout=PIPE, stderr=PIPE)
      self.returnCode = None
      while pp.poll() == None:
        print 'polling from popen...'
        line = pp.stdout.readline()
        if line != '':
          self.info(line)

        line = pp.stderr.readline()
        if line != '':
          self.error(line)
        self.version += 1
      if pp.returncode != 0:
        self.error('Error generating nvmdefault.h')
        return False
      self.info('Finishing compression')
      if  SIMULATION !=0:
          #we don't do any real deployment for simulation, 
          return False
      comm = getComm()
      # Deploy nvmdefault.h to nodes
      print 'changing to path: %s...' % platform_dir
      self.info('Deploying to nodes')
      for node_id in destination_ids:
        self.info('Deploying to node id: %d' % (node_id))
        if not comm.reprogram(node_id, os.path.join(platform_dir, 'nvmdefault.h'), retry=False):
          self.error('Node not deployed successfully')
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
        self.info('Deploying to node completed')
    self.info('Deployment has completed')

  def reconfiguration(self):
    node_infos = getComm().getAllNodeInfos()
    locationTree = LocationTree(LOCATION_ROOT)
    locationTree.buildTree(node_infos)
    self.map(locationTree)
    self.deploy([info.nodeId for info in node_infos], DEPLOY_PLATFORMS)
