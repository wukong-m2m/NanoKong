from globals import *
import pynvc

def signal_handler(signal):
    print 'signal_handler'
    if signal == pynvc.GROUP_NOTIFY_NODE_FAILURE:
        if len(applications) > 0:
            active_application().reconfiguration()
