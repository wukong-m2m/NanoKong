from wkpf import *
from locationTree import LocationTree
from URLParser import *
import copy

def firstCandidate(app, locTree, queries):
    #input: nodes, WuObjects, WuLinks, WuClassDefs
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes
    print app.wuObjects
    for wuObject in app.wuObjects.values():
        candidateSet = set()

        locURLHandler = LocationURL(None, locTree)
        candidateSet = locTree.root.getAllNodes()

        '''
        if queries[i] == None:
            locURLHandler = LocationURL(None, locTree)
            candidateSet = locTree.root.getAllNodes()
        else:
            locURLHandler = LocationURL(queries[i], locTree)
            locURLHandler.parseURL()
            
            tmpSet = locURLHandler.solveParseTree(locURLHandler.parseTreeRoot)
            if len(tmpSet) > 0:
                candidateSet = tmpSet
            else:
                print 'Conditions for component '+str(len(candidateSet))+'(start form 0) too strict, no available candidate found'
                return False
        '''

        print 'candidateSet', list(candidateSet)[0]

        wuObject.setNodeId(list(candidateSet)[0])    #select the first candidate who satisfies the condiditon
        sensorNode = locTree.sensor_dict[list(candidateSet)[0]]
        sensorNode.initPortList(forceInit = False)
        portNo = sensorNode.reserveNextPort()
        if portNo == None:
            print 'all port in node', wuObject, 'occupied, cannot assign new port'
            return False
        wuObject.setPortNumber(portNo)
            
    return True

class WuApplication:
    def __init__(self, flowDom, outputDir=None, templateDir=None, componentXml=None):
        self.applicationDom = flowDom;
        self.applicationName = flowDom.getElementsByTagName('application')[0].getAttribute('name')

        self.destinationDir = outputDir
        self.templateDir = templateDir
        self.componentXml = componentXml

        self.wuClasses = {}
        self.wuObjects = {}
        self.wuLinks = []

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

    def mappingWithNodeList(self, locTree, queries,mapFunc=firstCandidate):
        #input: nodes, WuObjects, WuLinks, WuClassDefs
        #output: assign node id to WuObjects
        # TODO: mapping results for generating the appropriate instiantiation for different nodes
        return mapFunc(self, locTree, queries)

    def generateJava(self):
        print 'generate Java'
        print self.wuObjects
        jinja2_env = Environment(loader=FileSystemLoader([os.path.join(self.templateDir, 'jinja_templates')]))
        output = open(os.path.join(self.destinationDir, self.applicationName+".java"), 'w')
        jinja2_env.get_template('application.java').render(applicationName=self.applicationName, wuObjectList=self.wuObjectList, wuLinks=self.wuLinks)
        output.write(jinja2_env.get_template('application.java').render(applicationName=self.applicationName, wuObjectList=self.wuObjectList, wuLinks=self.wuLinks))
        output.close()

