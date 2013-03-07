# vim: ts=4 sw=4
import sys, os, traceback, copy
sys.path.append(os.path.abspath(".."))
from parser import *
from locationTree import *
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import simplejson as json
import logging, logging.handlers, wukonghandler
from collections import namedtuple
from locationParser import *
from wkpfcomm import *
from codegen import CodeGen
from xml2java.generator import Generator
import copy
from threading import Thread
import traceback
import time
import re
import StringIO
import shutil, errno
import datetime
from subprocess import Popen, PIPE, STDOUT

from configuration import *
from globals import *

# allcandidates are all node ids ([int])
def constructHeartbeatGroups(heartbeatgroups, routingTable, allcandidates):
  del heartbeatgroups[:]

  while len(allcandidates) > 0:
    heartbeatgroup = namedtuple('heartbeatgroup', ['nodes', 'period'])
    heartbeatgroup.nodes = []
    heartbeatgroup.period = 1
    if len(heartbeatgroup.nodes) == 0:
      heartbeatgroup.nodes.append(allcandidates.pop(0)) # should be random?
      pivot = heartbeatgroup.nodes[0]
      if pivot.id in routingTable:
        for neighbor in routingTable[pivot.id]:
          neighbor = int(neighbor)
          if neighbor in [x.id for x in allcandidates]:
            for candidate in allcandidates:
              if candidate.id == neighbor:
                heartbeatgroup.nodes.append(candidate)
                allcandidates.remove(candidate)
                break
      heartbeatgroups.append(heartbeatgroup)

# assign periods
def determinePeriodForHeartbeatGroups(components, heartbeatgroups):
  for component in components:
    for wuobject in component.instances:
      for group in heartbeatgroups:
        if wuobject.node_id in [x.id for x in group.nodes]:
          #group heartbeat is reactiontime divided by 2, then multiplied by 1000 to microseconds
          newperiod = int(component.reaction_time / 2.0 * 1000.0)
          if not group.period or (group.period and group.period > newperiod):
            group.period = newperiod
          break

def sortCandidates(wuObjects):
    nodeScores = {}
    for candidates in wuObjects:
      for node in candidates:
        if node[0] in nodeScores:
          nodeScores[node[0]] += 1
        else:
          nodeScores[node[0]] = 1

    for candidates in wuObjects:
      sorted(candidates, key=lambda node: nodeScores[node[0]], reverse=True)

def firstCandidate(logger, changesets, routingTable, locTree):
    #input: nodes, WuObjects, WuLinks, WuClassDefsm, wuObjects is a list of wuobject list corresponding to group mapping
    #output: assign node id to WuObjects
    # TODO: mapping results for generating the appropriate instiantiation for different nodes

    # construct and filter candidates for every component on the FBP (could be the same wuclass but with different policy)
    for component in changesets.components:
        locationquery = component.location
        mincandidates = component.group_size

        # filter by location
        locParser = LocationParser(locTree)
        try:
            tmpSet = locParser.parse(locationquery)
        except:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            logger.error('cannot find match for location query %s, so we will invalid the query and pick all by default', locationquery)
            logger.error('Locality conditions for component wuclass id "%s" are too strict; no available candidate found' % (wuObject[0].getWuClass().getId()))
            logging.error("location Tree")
            logging.error(locTree.printTree())
            set_wukong_status(str(exc_type)+str(exc_value))
            tmpSet = locTree.getAllAliveNodeIds()
        candidates = tmpSet


        if len(candidates) < mincandidates:
            msg = '[WARNING] there is no enough candidates %r for component %s, but mapper will continue to map' % (candidates, component)
            set_wukong_status(msg)
            logger.warning(msg)

        # construct wuobjects, instances of component
        for candidate in candidates:
            node = locTree.getNodeInfoById(candidate)
            wuclass = WuClass.where(name=component.type)[0]
            # prefers native implementation
            if wuclass.id in [x.id for x in node.wuclasses]:
                # has wuclass for native implementation
                if wuclass.id in [x.wuclass.id for x in node.wuobjects]:
                    # use existing native wuobject
                    for wuobject in node.wuobjects:
                        if wuobject.wuclass.id == wuclass.id:
                            component.instances.append(wuobject)
                            break
                else:
                    # create a new native wuobject
                    sensorNode = locTree.sensor_dict[node.id]
                    sensorNode.initPortList(forceInit = False)
                    port_number = sensorNode.reserveNextPort()
                    wuclass_alternate = wuclass
                    if wuclass.virtual:
                        wuclass_alternate = copy.deepcopy(wuclass)
                        wuclass_alternate.virtual = False
                        wuclass_alternate.save() # a new found wuclass
                    wuobject = WuObject(node.id, port_number, wuclass_alternate)
                    wuobject.save()
                    component.instances.append(wuobject)
            else:
                if wuclass.id in [x.wuclass.id for x in node.wuobjects]:
                    # use existing virtual wuobject
                    for wuobject in node.wuobjects:
                        if wuobject.wuclass.id == wuclass.id:
                            component.instances.append(wuobject)
                            break
                else:
                    # create a new virtual wuobject
                    sensorNode = locTree.sensor_dict[node.id]
                    sensorNode.initPortList(forceInit = False)
                    port_number = sensorNode.reserveNextPort()
                    wuclass_alternate = wuclass
                    if not wuclass.virtual:
                        wuclass_alternate = copy.deepcopy(wuclass)
                        wuclass_alternate.virtual = True
                        wuclass_alternate.save() # a new found wuclass
                    wuobject = WuObject(node.id, port_number, wuclass_alternate)
                    wuobject.save()
                    component.instances.append(wuobject)
        
        if len(component.instances) == 0:
          logger.error ('[ERROR] No avilable match could be found for component %s' % (component))
          return False

    # sort candidates
    # TODO: think about it, probably not going to used for this thesis
    #sortCandidates(changesets.components)

    # construct heartbeat groups plus period assignment
    allcandidates = set()
    for component in changesets.components:
        for wuobject in component.instances:
            allcandidates.add(Node.where(id=wuobject.node_id)[0])
    constructHeartbeatGroups(changetsets.heartbeatgroups, routingTable, allcandidates)
    determinePeriodForHeartbeatGroups(changesets.components, changesets.heartbeatgroups)
    logging.info('heartbeatGroups constructed, periods assigned')
    logging.info(changesets.heartbeatgroups)

    #delete and roll back all reservation during mapping after mapping is done, next mapping will overwritten the current one
    for component in changeset.components:
        for wuobj in component.instances:
            senNd = locTree.getSensorById(wuobj.node_id)
            for j in senNd.temp_port_list:
                senNd.port_list.remove(j)
            senNd.temp_port_list = []
    return True
