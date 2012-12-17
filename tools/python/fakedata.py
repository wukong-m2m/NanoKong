from wkpf import *
from locationTree import *
from configuration import *

rootpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
all_wuclasses = parseXML(os.path.join(rootpath, "ComponentDefinitions", "WuKongStandardLibrary.xml")).values()
all_wutypes = parseXML(os.path.join(rootpath, "ComponentDefinitions", "WuKongStandardLibrary.xml"), type='wutype').values()

simNodeInfos = []
locs = [u"/universal/CS_Building/3F/South_Corridor/Room336#(0,1,2)", u"/universal/CS_Building/3F/East_Corridor/Room318#(0,5,4)",
u"/universal/CS_Building/3F/South_Corridor/Room336#(0,1,3)", u"/universal/CS_Building/3F/East_Corridor/Room318#(0,6,4)"]
coords = [(0, 1, 2), (0, 5, 3), (0,1,3), (0, 6, 4)]

def genFakeData():	
	wuobjects = []
	global simNodeInfos
	simNodeInfos=[]
	global locs
	global coords
	
	count = 0
	for id in range(4):
		count = 0
		wuobjects = []
		for wuclass in all_wuclasses:
			wuobjects.append(WuObject(wuclass, 'testId'+str(count), 1, nodeId = id, portNumber=count))
			if count > 10:
				break
			count = count+1
		simNodeInfos.append(NodeInfo(nodeId = id,
			     wuClasses = all_wuclasses,
			     wuObjects = wuobjects,
			     location = locs[id]))
			
'''
locs = ["Boli_Building/3F/South_Corridor/Room336", "Boli_Building/3F/East_Corridor/Room318"]
coords = [(0, 1, 2), (0, 5, 3)]

for node_info, loc, coord in zip(node_infos, locs, coords):
    locTree.addSensor(SensorNode(node_info, coord[0], coord[1], coord[2]))

queries = ["Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)|near(1,1,3,1)",
          "Boli_Building/3F/South_Corridor/Room336#near(0,1,2,1)|near(1,1,3,1)",
          None, None]
'''

if SIMULATION == 1:
    genFakeData()
