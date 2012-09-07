# vim: ts=2 sw=2

from wkpf import *
from locationTree import *
from xml.dom.minidom import parse
import simplejson as json
import logging
import logging.handlers
import wukonghandler
import copy
sys.path.append(os.path.abspath("../tools/xml2java"))
from URLParser import *

OK = 0
NOTOK = 1

def firstCandidate(app, wuObjects, locTree, queries):
    #input: nodes, WuObjects, WuLinks, WuClassDefs
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes
    print wuObjects

    for i, wuObject in enumerate(wuObjects.values()):
        candidateSet = set()

        if queries[i] == None:
            locURLHandler = LocationURL(None, locTree)
            candidateSet = locTree.root.getAllNodes()
        else:
            locURLHandler = LocationURL(queries[i], locTree)
            try:
              locURLHandler.parseURL()
            except Exception as e:
              app.error(e)
              return False
            
            tmpSet = locURLHandler.solveParseTree(locURLHandler.parseTreeRoot)
            if len(tmpSet) > 0:
                candidateSet = tmpSet
            else:
                app.error('Conditions for component ', str(len(candidateSet)), '(start form 0) too strict, no available candidate found')
                return False

        print 'candidateSet', list(candidateSet)[0]

        wuObject.setNodeId(list(candidateSet)[0])    #select the first candidate who satisfies the condiditon
        sensorNode = locTree.sensor_dict[list(candidateSet)[0]]
        sensorNode.initPortList(forceInit = False)
        portNo = sensorNode.reserveNextPort()
        if portNo == None:
            app.error('all port in node %s occupied, cannot assign new port' % (wuObject))
            return False
        wuObject.setPortNumber(portNo)
        
    return True

class WuApplication:
  def __init__(self, id='', name='', desc='', file='', dir='', flowDom=None, outputDir=None, templateDir=None, componentXml=None):
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
    self.mapping_results = {}
    self.mapper = None
    self.inspector = None
    # 5 levels: self.logger.debug, self.logger.info, self.logger.warn, self.logger.error, self.logger.critical
    self.logger = logging.getLogger('wukong')
    self.logger.setLevel(logging.DEBUG) # to see all levels
    self.loggerHandler = wukonghandler.WukongHandler(1024 * 3, target=logging.FileHandler(os.path.join(self.dir, 'compile.log')))
    self.logger.addHandler(self.loggerHandler)

    # For Mapper
    if flowDom:
      self.applicationDom = flowDom;
      self.applicationName = flowDom.getElementsByTagName('application')[0].getAttribute('name')

    if outputDir:
      self.destinationDir = outputDir
    if templateDir:
      self.templateDir = templateDir
    if componentXml:
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
              wuProperty.setDefault(propertyTag.getAttribute('default'))

          # nodeId is not used here, portNumber is generated later
          wuObj = WuObject(wuClass=wuClass, instanceId=componentTag.getAttribute('instanceId'), instanceIndex=index)

          self.wuObjects[wuObj.getInstanceId()] = wuObj

      # links
      for linkTag in self.applicationDom.getElementsByTagName('link'):
          fromWuObject = self.wuObjects[linkTag.parentNode.getAttribute('instanceId')]
          fromPropertyId = fromWuObject.getPropertyByName(linkTag.getAttribute('fromProperty')).getId()

          toWuObject = self.wuObjects[linkTag.getAttribute('toInstanceId')]
          toPropertyId = toWuObject.getPropertyByName(linkTag.getAttribute('toProperty')).getId()

          self.wuLinks.append( WuLink(fromWuObject.getInstanceId(), fromPropertyId, toWuObject.getInstanceId(), toPropertyId) )

  def mappingWithNodeList(self, locTree, queries, mapFunc=firstCandidate):
      #input: nodes, WuObjects, WuLinks, WuClassDefs
      #output: assign node id to WuObjects
      # TODO: mapping results for generating the appropriate instiantiation for different nodes
      return mapFunc(self, self.wuObjects, locTree, queries)
