from wkpf.globals import *
from wkpf.wubutler import *
import gevent
from gevent.event import AsyncResult
import wkpf.pynvc
import logging

signal_reconfiguration = False
signal_deployment = None

def signal_reconfig():
    global signal_reconfiguration
    logging.info('reconfiguration signal set')
    signal_reconfiguration = True

def signal_deploy(*args):
    global signal_deployment
    logging.info('deploy signal set')
    signal_deployment = args

def signal_handler():
    global signal_reconfiguration
    global signal_deployment
    while 1:
        if not is_master_busy():
            if signal_reconfiguration:
                if len(applications) > 0:
                    WuButler().current_application.reconfiguration()
                    signal_reconfiguration = False

            if signal_deployment:
                if len(applications) > 0:
                    gevent.sleep(2)
                    WuButler().current_application.deploy_with_discovery(*signal_deployment)
                    #node_ids = [info.nodeId for info in getComm().getActiveNodeInfos(force=True)]
                    signal_deployment = None

        gevent.sleep(0.15)
