# vim: ts=2 sw=2

import sys, os
sys.path.append(os.path.abspath("../xml2java"))
sys.path.append(os.path.abspath("../../master"))
from wkpf import *
from locationTree import *
from xml.dom.minidom import parse
import simplejson as json
import logging
import logging.handlers
import wukonghandler
import copy
from URLParser import *
from wkpfcomm import *
from codegen import generateCode

from configuration import *

OK = 0
NOTOK = 1

def firstCandidate(app, wuObjects, locTree):
    #input: nodes, WuObjects, WuLinks, WuClassDefs
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes
    print wuObjects

    for i, wuObject in enumerate(wuObjects.values()):
        candidateSet = set()
        queries = wuObject.getLocationQueries()

        # filter by location query
        print queries
        if queries == []:
            locURLHandler = LocationURL(None, locTree)
            candidateSet = locTree.root.getAllNodes()
        else:
            print 'LocationURL', queries[0], locTree
            locURLHandler = LocationURL(queries[0], locTree) # get the first location query for a component, TODO:should consider other queries too later

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
                logging.err('Locality conditions for component "%s" are too strict; no available candidate found' % (wuObject.getInstanceId()))
                return False

        # filter by available wuclasses for non-virtual components
        node_infos = [locTree.getNodeInfoById(nodeId) for nodeId in candidateSet]
        if not wuObject.getWuClass().isVirtual():
          candidateSet = [node_info.nodeId for node_info in node_infos if wuObject.getWuClassId() in [wuClass.getId() for wuClass in node_info.wuClasses]]
        else:
          candidateSet = list(candidateSet)

        if len(candidateSet) == 0:
          app.error('No nodes could be mapped for component %s' % (wuObject.getInstanceId()))
          return False
        
        print 'will select the first in this candidateSet', candidateSet

        wuObject.setNodeId(candidateSet[0])    #select the first candidate who satisfies the condiditon
        sensorNode = locTree.sensor_dict[list(candidateSet)[0]]
        sensorNode.initPortList(forceInit = False)
        portNo = sensorNode.reserveNextPort()
        if portNo == None:
            app.error('All ports in node %s occupied, cannot assign new port' % (candidateSet[0]))
            return False
        wuObject.setPortNumber(portNo)
        
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
          for locationQuery in componentTag.getElementsByTagName('location'):
              queries.append(locationQuery.getAttribute('requirement'))

          # nodeId is not used here, portNumber is generated later
          wuObj = WuObject(wuClass=wuClass, instanceId=componentTag.getAttribute('instanceId'), instanceIndex=index, locationQueries=queries)

          self.wuObjects[wuObj.getInstanceId()] = wuObj

      # links
      for linkTag in self.applicationDom.getElementsByTagName('link'):
          fromWuObject = self.wuObjects[linkTag.parentNode.getAttribute('instanceId')]
          fromPropertyId = fromWuObject.getPropertyByName(linkTag.getAttribute('fromProperty')).getId()

          toWuObject = self.wuObjects[linkTag.getAttribute('toInstanceId')]
          toPropertyId = toWuObject.getPropertyByName(linkTag.getAttribute('toProperty')).getId()

          self.wuLinks.append( WuLink(fromWuObject, fromPropertyId, toWuObject, toPropertyId) )

  def map(self, location_tree):
    self.parseComponents()
    self.parseApplicationXML()
    self.mapping(location_tree)
    self.mapping_results = self.wuObjects

  def mapping(self, locTree, mapFunc=firstCandidate):
      #input: nodes, WuObjects, WuLinks, WuClassDefs
      #output: assign node id to WuObjects
      # TODO: mapping results for generating the appropriate instiantiation for different nodes
      
      return mapFunc(self, self.wuObjects, locTree)

  def deploy(self, destination_ids, platforms):
    comm = getComm()
    app_path = self.dir
    for platform in platforms:
      platform_dir = os.path.join(app_path, platform)

      # CodeGen
      self.info('Generating necessary files for wukong')
      try:
        generateCode(self)
      except Exception as e:
        traceback.print_last()
        self.error(e)
        return False

      # Mapper results, already did in map_application
      # Generate java code
      self.info('Generating application code in target language (Java)')
      try:
        generateJava(self)
      except Exception as e:
        traceback.print_last()
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
