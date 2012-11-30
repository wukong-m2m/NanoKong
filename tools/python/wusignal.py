from globals import *
import gevent
import pynvc
import logging

signal_reconfiguration = False

def signal_reconfig():
    global signal_reconfiguration
    logging.info('reconfiguration signal set')
    signal_reconfiguration = True

def signal_handler():
    global signal_reconfiguration
    while 1:
        if signal_reconfiguration:
            if len(applications) > 0:
                active_application().reconfiguration()
                signal_reconfiguration = False

        gevent.sleep(0)
