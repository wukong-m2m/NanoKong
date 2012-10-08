import os
from configobj import ConfigObj
ZWAVE_GATEWAY_IP = None

CONFIG_PATH = os.path.join(os.path.abspath(os.path.dirname(__file__)), '..', 'config', 'master.cfg')
print CONFIG_PATH

def readConfig():
    config = ConfigObj(CONFIG_PATH)
    print config
    #get zwave_ip from network section
    global ZWAVE_GATEWAY_IP 
    ZWAVE_GATEWAY_IP = config.get('ZWAVE_GATEWAY_IP')
    print "zwave gateway IP: "+ZWAVE_GATEWAY_IP

readConfig()
