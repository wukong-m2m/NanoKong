#!/usr/bin/env python
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../master'))

from wkpf.wubutler import *
from wkpf.configuration import LOCATION_ROOT
from wkpf.globals import *
from wkpf.wkpfcomm import *
from wkpf.parser import *

Parser.parseLibrary(COMPONENTXML_PATH)
WuButler().load_applications()
print [app.config() for app in WuButler().applications]
master_busy()
node_infos = getComm().getActiveNodeInfos(force=True)
print [info.location for info in node_infos]
raw_input('Press to continue...')
routingTable = getComm().getRoutingInformation()
location_tree = LocationTree(LOCATION_ROOT)
location_tree.buildTree(node_infos)
WuButler().applications[0].map(location_tree, routingTable)
#WuButler().applications[0].deploy([info.id for info in node_infos], DEPLOY_PLATFORMS)
master_available()
