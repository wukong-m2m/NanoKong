import os
from configobj import ConfigObj
ZWAVE_GATEWAY_IP = None
MASTER_PORT = 80
LOCATION_ROOT = "universal"
CONFIG_PATH = os.path.join(os.path.abspath(os.path.dirname(__file__)), '..', 'config', 'master.cfg')
print CONFIG_PATH

def readConfig():
    config = ConfigObj(CONFIG_PATH)
    print config
    #get zwave_ip from network section
    global ZWAVE_GATEWAY_IP 
    global MASTER_PORT
    ZWAVE_GATEWAY_IP = config.get('ZWAVE_GATEWAY_IP')
    MASTER_PORT = int(config.get('MASTER_PORT'))
    
    global LOCATION_ROOT
    LOCATION_ROOT = config.get('LOCATION_ROOT')
readConfig()
