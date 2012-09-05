from wkpf import *
from wkpfcomm import Communication
from inspector import Inspector
from locationTree import LocationTree, SensorNode, parseLocation

rootpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
wuclasses = parseXML(os.path.join(rootpath, "ComponentDefinitions", "WuKongStandardLibrary.xml")).values()

node_infos = [NodeInfo(nodeId=3,
                wuClasses=wuclasses,
                wuObjects=[])]

locTree = LocationTree("Boli_Building")
locs = ["Boli_Building/3F/South_Corridor/Room336", "Boli_Building/3F/East_Corridor/Room318"]
coords = [(0, 1, 2), (0, 5, 3)]

for node_info, loc, coord in zip(node_infos, locs, coords):
    locTree.addSensor(SensorNode(parseLocation(loc), node_info, coord[0], coord[1], coord[2]))

queries = ["Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)|near(1,1,3,1)",
          "Boli_Building/3F/South_Corridor/Room336#near(0,1,2,1)|near(1,1,3,1)",
          None, None]
