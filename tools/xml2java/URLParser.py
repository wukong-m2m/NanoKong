#Sen Zhou 12.8.14 
#define how to parse URLs like EE_Building/3F/ROOM318#near(0,1,2)&inside(3F),  
#(, ), &, |, are preserved key words
#priority | < & < ~
#any kind of spaces, tabs or "\n" are not allowed in URL

import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "../python"))
from wkpf import WuClass, WuObject, NodeInfo
from locationTree import *


class LocationParseTreeNode(object):
    def __init__(self, contentStr):
        self.function = None
        self.childCnt = 0
        self.operation = None
        self.children = []
        self.contentStr = contentStr

class LocationURL(object):
    def __init__(self, urlStr, locationTree):
        self.locationTreeNode = None
        self.urlStr = urlStr
        self.parseTreeRoot = None
        self.nodeIdSet = set([]) 
        self.locationTree = locationTree

    def near(locationTreeNode, x,y,z,dist):
        ret_val = set([])
        print x,y,z, dist
        x = float(x)
        y = float(y)
        z = float(z)
        dist = float(dist)
        print 'near', locationTreeNode
        for sensorNd in locationTreeNode.sensorLst:
            if (sensorNd.coord[0]-x)**2+(sensorNd.coord[1]-y)**2+(sensorNd.coord[2]-z)**2 <= dist**2:
                print sensorNd.coord[0],sensorNd.coord[1],sensorNd.coord[2]
                ret_val.add(sensorNd.nodeInfo.nodeId)
        return ret_val

    def exact(locationTreeNode, id):
        id = int(id)
        return set([id])

    def getIntersect(locationTreeNode,  set1, set2):
        return set1 & set2

    def getUnion(locationTreeNode,  set1, set2):
        return set1 | set2

    def negate(locationTreeNode, set1):
        return locationTreeNode.idSet - set1

    def getAll(locationTreeNode):
        return locationTreeNode.getAllNodes()

    connector_lst=[ "|", "&", "~"]
    connector_dict = {"|":[getUnion,2], "&":[getIntersect,2], "~":[negate,1]}  #for DNF(disjunctive norm form expression), function call and num of variables
    funct_dict = {"near":near, "exact":exact}



    def _isFuncParameter(self, numStr):
        try:
            numPara = float(numStr)
        except ValueError:
            return False
        return True

    def _isFuncName(self, nameStr):
        if self.funct_dict.has_key(nameStr):
            return True
        return False

    def _genFunction(self,functionStr):
        tmpLst = functionStr.split('(')
        func_name = tmpLst[0].strip()
        foo = URLFunction(func_name, self.locationTreeNode)
        if self._isFuncName(func_name) == False:
            print "Error while parsing url:" + self.urlStr +", no function named "+ func_name+" found!"
        try:
            foo.func = self.funct_dict[func_name]
        except KeyError:
            print "No function named"+ func_name+" found"
            return None
        foo.args += tmpLst[1].strip(" ),").split(',')
        return foo

    #parameters: str --- string to be analysis, connector_id ---- seq. of connector in connector_lst
    def _buildParseTree(self, treeNode, connector_id):
        if connector_id >= len(self.connector_lst):		#parse to single functions
            if len(treeNode.contentStr)>0:
                funct = self._genFunction(treeNode.contentStr)
                treeNode.function = funct
                return 0
            else:
                return -1

        location = treeNode.contentStr.find(self.connector_lst[connector_id])
        if location == -1:
            self._buildParseTree(treeNode, connector_id+1)
        else:
            treeNode.operation = self.connector_lst[connector_id]
            if location != 0:
                treeNode.children.append(LocationParseTreeNode(treeNode.contentStr[:location]))
                print treeNode.contentStr[:location], connector_id
                self._buildParseTree(treeNode.children[-1], connector_id+1)
            treeNode.children.append(LocationParseTreeNode(treeNode.contentStr[location+len(self.connector_lst[connector_id]):]))
            print treeNode.contentStr[location+len(self.connector_lst[connector_id]):]
            self._buildParseTree(treeNode.children[-1], connector_id)


    def solveParseTree(self):
        if not self.parseTreeRoot:
            return self.nodeIdSet

        if self.parseTreeRoot.function!=None:
            return self.parseTreeRoot.function.runFunction()

        if self.connector_dict[self.parseTreeRoot.operation][1] == 1:	#unary op
            return self.connector_dict[self.parseTreeRoot.operation][0](self.locationTreeNode, self.solveParseTree(self.parseTreeRoot.children[0]))

        elif self.connector_dict[self.parseTreeRoot.operation][1] == 2:	#binary op
            return self.connector_dict[self.parseTreeRoot.operation][0](self.locationTreeNode,  self.solveParseTree(self.parseTreeRoot.children[0]), self.solveParseTree(self.parseTreeRoot.children[1]))

        else:
            print "Error when parsing tree, no operations has more than 2 operands"
            return None


    #generate parse tree, add URLFunction objects into the parse tree, return tree root
    def _parse2Functions(self, functionLstStr):
        trNd = LocationParseTreeNode(functionLstStr)
        self._buildParseTree(trNd, 0)
        self.parseTreeRoot = trNd
        return trNd

    def _createNodeIdSet(self):
        if self.locationTreeNode != None:
            self.nodeIdSet = self.locationTreeNode.idSet
        else:
            raise Exception('Error! No location tree')

    #be able to parse sth like near(0,1,2)
    def parseURL(self):
        tmpStrLst = self.urlStr.split('#')
        if(len(tmpStrLst) > 2):
            print "Error while parsing url:" + self.urlStr +", more than one # found!"
            return False

        self.locationTreeNode = self.locationTree.findLocation(self.locationTree.root, tmpStrLst[0])
        if not self.locationTreeNode:
            raise Exception("Error! location cannot be found: "+ str(locationLst))

        if len(tmpStrLst) > 1:
            self._parse2Functions(tmpStrLst[1])

        self._createNodeIdSet()


class URLFunction(object):
    def __init__(self, name, locationTreeNode):
        self.name = name
        self.func = None
        self.args = [locationTreeNode]
    def addParameter(self, para):
        self.args += para
    def runFunction(self):
        return self.func(*self.args)



if __name__ == "__main__":
    locTree = LocationTree("Boli_Building")
    loc0 = "Boli_Building/3F/South_Corridor/Room318"
    loc1 = "Boli_Building/3F/South_Corridor/Room318"
    loc2 = "Boli_Building/3F/South_Corridor/Room318"
    loc3 = "Boli_Building/3F/South_Corridor/Room318"
    senNd0 = SensorNode(NodeInfo(0, [], [], location=loc0), 0, 1, 1)
    senNd1 = SensorNode(NodeInfo(1, [], [], location=loc1), 0, 1, 3)
    senNd2 = SensorNode(NodeInfo(2, [], [], location=loc2), 1, 1, 2)
    senNd3 = SensorNode(NodeInfo(3, [], [], location=loc3), 4, 4, 2)
    locTree.addSensor(locTree.root, senNd0)
    locTree.addSensor(locTree.root, senNd1)
    locTree.addSensor(locTree.root, senNd2)
    locTree.addSensor(locTree.root, senNd3)
    locTree.printTree(locTree.root, 0)
    query = "Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)&~near(1,1,3,1)|exact(0)"
    locURLHandler = LocationURL(query, locTree)
    locURLHandler.parseURL()
#	print locURLHandler.locationTreeNode.sensorLst
    print locURLHandler.solveParseTree()



