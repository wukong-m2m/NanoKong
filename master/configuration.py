import os
from configobj import ConfigObj

ROOT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
CONFIG_PATH = os.path.join(ROOT_PATH, 'config', 'master.cfg')
config = ConfigObj(CONFIG_PATH)

ZWAVE_GATEWAY_IP = config.get('ZWAVE_GATEWAY_IP', '')
MASTER_PORT = int(config.get('MASTER_PORT', 80))

LOCATION_ROOT = config.get('LOCATION_ROOT', 'universal')

DEPLOY_PLATFORMS = ['avr_mega2560']

#XML_PATH = os.path.join(ROOT_PATH, 'Applications')
COMPONENTXML_PATH = os.path.join(ROOT_PATH, 'ComponentDefinitions', 'WuKongStandardLibrary.xml')
TEMPLATE_DIR = os.path.join(ROOT_PATH, 'tools', 'xml2java')
JAVA_OUTPUT_DIR = os.path.join(ROOT_PATH, 'java', 'examples')
TESTRTT_PATH = os.path.join(ROOT_PATH, 'tools', 'python', 'pyzwave')
APP_DIR = os.path.join(ROOT_PATH, 'vm', 'apps')
BASE_DIR = os.path.join(APP_DIR, 'base')
