from wkpf import *
from wkpfcomm import Communication
from inspector import Inspector
from locationTree import LocationTree, SensorNode, parseLocation

node_infos = [NodeInfo(nodeId=1, wuClasses=[], wuObjects=[]), NodeInfo(nodeId=3, 
    wuClasses=[
      WuClass('testClass', 0, {}, False, False, 3),
      WuClass(name='secondClass', id=1, properties={'testProperty': WuProperty('secondClass', 'testProperty', 0, WuType('boolean', 'boolean'), 'readonly')}, virtual=False, soft=False, node_id=3)], 
    wuObjects=[
      WuObject(wuClass=WuClass('secondClass', 1, {'testProperty': WuProperty('secondClass', 'testProperty', 0, WuType('boolean', 'boolean'), 'readonly')}, False, False), 
        instanceId='secondClass0', instanceIndex=1, nodeId=3, portNumber=12)])]

locTree = LocationTree("Boli_Building")
locs = ["Boli_Building/3F/South_Corridor/Room336", "Boli_Building/3F/East_Corridor/Room318"]
coords = [(0, 1, 2), (0, 5, 3)]

for node_info, loc, coord in zip(node_infos, locs, coords):
    locTree.addSensor(SensorNode(parseLocation(loc), node_info, coord[0], coord[1], coord[2]))

queries = ["Boli_Building/3F/South_Corridor/Room318#near(0,1,2,1)|near(1,1,3,1)",
          "Boli_Building/3F/South_Corridor/Room336#near(0,1,2,1)|near(1,1,3,1)",
          None, None]
