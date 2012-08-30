from wkpf import *
from locationTree import LocationTree
from URLParser import *

class WuApplication:
    def __init__(self, flowDom, outputDir, componentDir, rootDir):
        self.applicationDom = flowDom;
        self.applicationName = flowDom.getElementsByTagName('application')[0].getAttribute('name')
        self.destinationDir = outputDir
        self.componentDir = componentDir
        self.templateDir = os.path.join(rootDir, 'tools', 'xml2java')
        self.wuClassDefs = None
        self.wuObjectList = WuObjectDefList()
        self.wuLinks = []

    def parseComponents(self):
        componentParser = WuClassXMLParser(self.componentDir)
        self.wuClassDefs = componentParser.parse() # wuClassDefs of components

    def scaffoldingWithComponents(self):
        # TODO: parse application XML to generate WuClasses, WuObjects and WuLinks
        wuObj = None
        for componentTag in self.applicationDom.getElementsByTagName('component'):
            # make sure application component is found in wuClassDef component list
 #           assert componentTag.getAttribute('type') in self.wuClassDefs   #Sen12.8.22 assertion type not compatible, need revision

            wuClassDef = self.wuClassDefs.getByTypeName(componentTag.getAttribute('type'))
            assert wuClassDef != None
            # nodeId is not used here, portNumber is generated later
            wuObj = WuObject(portNumber=None, wuClass=WuClass(None,wuClassDef.id, wuClassDef = wuClassDef), instanceId=componentTag.getAttribute('instanceId'))
            print wuObj.wuClass.wuClassDef.properties
            # TODO: for java variable instantiation
            for propertyTag in componentTag.getElementsByTagName('property'):
                assert propertyTag.getAttribute('name') in wuClassDef
               # wuObject = self.wuObjectList.getByTypeName(wuClassDef.getTypeName())
                wuProperty = wuObj.getPropertyByName(propertyTag.getAttribute('name'))
                if wuProperty.getType() != 'int' and wuProperty.getType() != 'boolean' and wuProperty.getType() != 'short' and wuProperty.getType() != 'refresh_rate':
                    wuProperty.default_value = wuProperty.getJavaNameForValue(propertyTag.getAttribute('default'))
                else:
                    wuProperty.default_value = propertyTag.getAttribute('default')
                print wuProperty
            self.wuObjectList.append(wuObj)

        # links
        for linkTag in self.applicationDom.getElementsByTagName('link'):
            fromWuObject = self.wuObjectList.getByInstanceId(linkTag.parentNode.getAttribute('instanceId'))
            fromProperty = fromWuObject.getPropertyByName(str(linkTag.getAttribute('fromProperty'))).getId()

            print 'links', self.wuObjectList
            print linkTag.getAttribute('toInstanceId')
            toWuObject = self.wuObjectList.getByInstanceId(str(linkTag.getAttribute('toInstanceId')))
            toProperty = toWuObject.getPropertyByName(str(linkTag.getAttribute('toProperty'))).getId()

            self.wuLinks.append( WuLink(fromWuObject, fromProperty, toWuObject, toProperty) )
    def firstCandidate(app, locTree, queries):
        #input: nodes, WuObjects, WuLinks, WuClassDefs
        #output: assign node id to WuObjects
        # TODO: mapping results for generating the appropriate instiantiation for different nodes
        candidateSets = []
        for i in range(len(app.wuObjectList)):
            if queries[i] == None:
                locURLHandler = LocationURL(None, locTree)
                candidateSets.append(locTree.root.getAllNodes())
                continue
            locURLHandler = LocationURL(queries[i], locTree)
            locURLHandler.parseURL()
            
            tmpSet = locURLHandler.solveParseTree(locURLHandler.parseTreeRoot)
            if len(tmpSet) >0:
                candidateSets.append(tmpSet)
            else:
                print 'Conditions for component '+str(len(candidateSets))+'(start form 0) too strict, no available candidate found'
                return False
        for i in range(len(app.wuObjectList)):
            app.wuObjectList[i].setNodeId(tuple(candidateSets[i])[0])    #select the first candidate who satisfies the condiditon
            sensorNode = locTree.sensor_dict[tuple(candidateSets[i])[0]]
            sensorNode.initPortList(forceInit = False)
            portNo = sensorNode.reserveNextPort()
            if portNo == None:
                print 'all port in node', i, 'occupied, cannot assign new port'
                return False
            app.wuObjectList[i].setPortNumber(portNo)
                
        return True

    def mappingWithNodeList(self, locTree, queries,mapFunc = firstCandidate):
        #input: nodes, WuObjects, WuLinks, WuClassDefs
        #output: assign node id to WuObjects
        # TODO: mapping results for generating the appropriate instiantiation for different nodes
        ret = mapFunc(self, locTree, queries)
        assert ret==True

    def generateJava(self):
        print 'inside generate Java'
        print self.wuObjectList.wuobjects
        jinja2_env = Environment(loader=FileSystemLoader([os.path.join(self.templateDir, 'jinja_templates')]))
        output = open(os.path.join(self.destinationDir, self.applicationName+".java"), 'w')
        jinja2_env.get_template('application.java').render(applicationName=self.applicationName, wuObjectList=self.wuObjectList, wuLinks=self.wuLinks)
        output.write(jinja2_env.get_template('application.java').render(applicationName=self.applicationName, wuObjectList=self.wuObjectList, wuLinks=self.wuLinks))
        output.close()