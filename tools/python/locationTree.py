#the children of leaf nodes are sensor nodes, sensor nodes

import logging
import odict
import json
from wkpf import NodeInfo, WuClass, WuObject

json_data = odict.odict()
#json_data = {}
number = 0

class SensorNode:
    def __init__(self, nodeInfo, x_coord, y_coord, z_coord):
        self.location = nodeInfo.location
        self.locationLst = LocationTree.parseLocation(nodeInfo.location)
        self.locationTreeNode = None
        self.nodeInfo = nodeInfo
        self.coord = (x_coord,y_coord,z_coord)
        self.port_list = []
        
    def initPortList(self, forceInit = True):
        if len(self.port_list)!=0 and forceInit == False:
            return
        for wuObj in self.nodeInfo.wuObjects:
            self.port_list.append(wuObj.getPortNumber())
        self.port_list.sort()
    def reserveNextPort(self):
        portSet = False
        for j in range(len(self.port_list)):
            if (self.port_list[j]+1)%256 !=self.port_list[(j+1)%len(self.port_list)]:
                self.port_list.append((self.port_list[j]+1)%256)
                self.port_list.sort()
                portSet =True
                return (self.port_list[j]+1)%256
        return None
        
class LocationTreeNode:
	def __init__(self, name, parent):
		self.name = name
		self.parent = parent
		self.children = []
		self.childrenCnt = 0
		self.sensorLst = []
		self.sensorCnt = 0
		self.idSet = set([])			#all sensor ids contained in this Node and its children nodes
	def addChild(self, name):
		tmp = LocationTreeNode (name, self)
		self.children.append(tmp)
		self.childrenCnt = self.childrenCnt + 1
		
	def delChild(self, locTreeNode):
		self.children.remove(locTreeNode)
		self.childrenCnt = self.childrenCnt - 1
		
	def addSensor(self, sensorNode):
		self.sensorLst.append(sensorNode)
		self.sensorCnt = self.sensorCnt + 1
		sensorNode.locationTreeNode = self
		self.idSet.add(sensorNode.nodeInfo.nodeId)
		# update sensorCnt for all ancestor nodes
		pa = self.parent
		while pa != None:
			pa.sensorCnt = pa.sensorCnt + 1
			pa.idSet.add(sensorNode.nodeInfo.nodeId)
			pa = pa.parent
	
	def delSensor(self, sensorNode):
		self.sensorLst.remove(sensorNode)
		self.sensorCnt = self.sensorCnt - 1
		sensorNode.locationTreeNode = None
		self.idSet.remove(sensorNode.nodeInfo.nodeId)
		# update sensorCnt for all ancestor nodes
		pa = self.parent
		while pa != None:
			pa.sensorCnt = pa.sensorCnt - 1
			pa.idSet.remove(sensorNode.nodeInfo.nodeId)
			pa = pa.parent
		
	def getAllNodes(self):
		ret_val = self.idSet
		return ret_val
	
	def getAllNodeInfos(self):
		ret_val = []
		for sensor in self.sensorLst:
			ret_val.append(sensor.nodeInfo)
		for child in self.children:
			ret_val.append(child.getAllNodeInfos())
		return ret_val
		
	def toString(self, indent = 0):
		print_str = ""
		for i in range(indent):
			print_str  = print_str + "\t"
		print_str = print_str + self.name + "#"
		for i in range(len(self.sensorLst)):
			print_str = print_str + str(self.sensorLst[i].nodeInfo.nodeId) +str(self.sensorLst[i].coord)+", "
		return print_str
		
	def _toString(self, indent = 0):
		global number
		print_str = ""
		print_str = print_str + self.name  + "#"
#		for i in range(len(self.sensorLst)):
#			print_str = print_str + str(self.sensorLst[i].nodeInfo.nodeId) +str(self.sensorLst[i].coord)+", "
		json_data[indent+number*10] = print_str
#		json_data[print_str] = indent + number*10
		if not len(self.sensorLst) == 0:		
			self._chldString(indent)
		return print_str
		
	def _chldString(self, indent=0):
		global number
		for i in range(len(self.sensorLst)):
			number += 1
#			json_data[str(self.sensorLst[i].nodeInfo.nodeId) + str(self.sensorLst[i].coord)] = indent+1+number*10
			json_data[indent+1+number*10] = str(self.sensorLst[i].nodeInfo.nodeId) + str(self.sensorLst[i].coord)
			
		
		
class LocationTree:
        
    def __init__(self, name):
        tmp = LocationTreeNode(name, None)
        self.sensor_dict = {}
        self.root = tmp
        self.totalSensorCount = 0
    
    def delSensor(self, sensorId):
        if sensorId not in self.sensor_dict:
            logging.info("Node",sensorId," not in location tree, deletion ignored")
        sensorNode = self.sensor_dict[sensorId]
        locTreeNode = sensorNode.locationTreeNode
        
        locTreeNode.delSensor(sensorNode)
        del self.sensor_dict[sensorId]
        self.totalSensorCount = self.totalSensorCount - 1
        #delete unnecessary branches in the tree (del branches with no sensor node)
        while locTreeNode.sensorCnt == 0:
            pa = locTreeNode.parent
            if pa != None:
                pa.delChild(locTreeNode)
            del locTreeNode
            locTreeNode = pa
    
    def getAllNodeInfos(self):
        return self.root.getAllNodeInfos()


    #insert sensorNd into the tree with its location specified in locationLst, starting from startPos node(set to root if locationLst start from beginning)
    def addSensor(self, sensorNd, startPos = None ):
        if startPos == None:
            startPos = self.root
        if sensorNd.nodeInfo.nodeId in self.sensor_dict:
            if  sensorNd.nodeInfo.location == self.sensor_dict[sensorNd.nodeInfo.nodeId].location:
                logging.info("Node",sensorNd.nodeInfo.nodeId," already inserted, insertion ignore")
                return False
            else: #sensor node location needs to be updated, delete the original inserted SensorNd first
                self.delSensor(sensorNd.nodeInfo.nodeId)
        if sensorNd.locationLst == None or len(sensorNd.locationLst) == 0:
            logging.error("error! location for node "+ str(sensorNd.nodeInfo.nodeId)+ " is not set")
            return False
        if startPos.name != sensorNd.locationLst[0]:
            logging.error("error! location: "+ str(sensorNd.locationLst[0])+ " does not match " + startPos.name)
            return False
        curPos = startPos
        for i in range(1, len(sensorNd.locationLst)):
            
            if  curPos.childrenCnt==0:
                curPos.addChild(sensorNd.locationLst[i])
                curPos = curPos.children[-1]
            else:
                child_index = -1
                for j in range(curPos.childrenCnt):
                    if curPos.children[j].name == sensorNd.locationLst[i]:
                        child_index = j
                if (child_index >= 0):
                    curPos = curPos.children[child_index]
                else:
                    curPos.addChild(sensorNd.locationLst[i])
                    curPos = curPos.children[-1]
                
        curPos.addSensor(sensorNd)
        self.sensor_dict[sensorNd.nodeInfo.nodeId] = sensorNd
        self.totalSensorCount = self.totalSensorCount +1
        return True
    
    def getNodeInfoById (self, Id):    			#return None on failure
        curNode = self.root
        while curNode != None and (Id in curNode.idSet):
            found = False
            for child in curNode.children:
                if Id in child.idSet:
                    curNode = child
                    found = True
                    break
            if found == False:		#sensor in curNode but not in its children
                for senr in curNode.sensorLst:
                    if senr.nodeInfo.nodeId == Id:
                        return senr.nodeInfo
                curNode = None
        return None
                
    def findLocation(self, startPos, locationStr):
        locationLst =  self.parseLocation(locationStr)
        print locationLst
        if startPos.name != locationLst[0]:
            logging.error("error! location: "+ str(locationLst[0])+ " is not a valid value")
            return None
        curPos = startPos

        for i in range(1, len(locationLst)):
            if  curPos.childrenCnt==0:
                return None
            else:
                child_index = -1
                for j in range(curPos.childrenCnt):
                    if curPos.children[j].name == locationLst[i]:
                            child_index = j
                if (child_index >= 0):
                    curPos = curPos.children[child_index]
                else:
                    return None
        return curPos
    @staticmethod
    def parseLocation (locationStr):
        locationLst = locationStr.split(u'/')
        for loc in locationLst:
            if len(loc) == 0:
                locationLst.remove(loc)
                
        return locationLst


    def __printTree(self, treeNd=None, indent = 0):
        global number
        number += 1

        str = ""
        _str = ""
        
        if treeNd ==None:
            treeNd = self.root

            print "Printing location tree!"

        str += treeNd.toString(indent)
        _str += treeNd._toString(indent)
        json_data[indent+number*10] = _str
        
        print (str)
        
        for i in range(treeNd.childrenCnt):			
            self.printTree(treeNd.children[i], indent+1)

    def printTree(self, treeNd=None, indent = 0):
        global number
        number += 1

        _str = ""
        if treeNd ==None:
            treeNd = self.root

        _str += treeNd._toString(indent)
#		json_data[indent+number*10] = _str

        print (_str)

        for i in range(treeNd.childrenCnt):
            self.printTree(treeNd.children[i], indent+1)

    def getJson(self):
        return json_data

    def buildTree(self, node_infos):
        for info in node_infos:
            senNd = SensorNode(info, 0, 0, 0)
            self.addSensor(senNd)


if __name__ == "__main__":
    locTree = LocationTree(u"Boli_Building")
    loc0 = u"Boli_Building/3F/South_Corridor"
    loc1 = u"Boli_Building/2F/South_Corridor/Room318"
    loc2 = u"Boli_Building/3F/East_Corridor/Room318"
    loc3 = u"Boli_Building/3F/East_Corridor/Room318"
    senNd0 = SensorNode(NodeInfo(0,[], [], loc0), 0, 1, 2)
    senNd1 = SensorNode(NodeInfo(1, [], [], loc1), 0, 5, 3)
    senNd2 = SensorNode( NodeInfo(2, [], [], loc2), 3, 3, 2)
    senNd3 = SensorNode(NodeInfo(3, [], [], loc3), 2, 1, 2)
    
    print(locTree.addSensor(senNd0, locTree.root))
    locTree.addSensor(senNd1)
    locTree.addSensor(senNd2)
    locTree.addSensor(senNd3)
    locTree.printTree(locTree.root, 0)
    locTree.delSensor(1)
    locTree.printTree()
    print json_data

    
            
        
