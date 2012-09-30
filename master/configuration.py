import ConfigParser
ZWAVE_GATEWAY_IP = None

def readConfig():
	config = ConfigParser.RawConfigParser()
	config.read('../config/master.cfg')
	#get zwave_ip from network section
	global ZWAVE_GATEWAY_IP 
	ZWAVE_GATEWAY_IP = config.get("network", 'ZWAVE_GATEWAY_IP')
	print "zwave gateway IP: "+ZWAVE_GATEWAY_IP
