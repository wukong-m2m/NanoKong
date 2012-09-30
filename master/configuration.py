from configobj import ConfigObj
ZWAVE_GATEWAY_IP = None

def readConfig():
	config = ConfigObj('../config/master.cfg')
	#get zwave_ip from network section
	global ZWAVE_GATEWAY_IP 
	ZWAVE_GATEWAY_IP = config.get('ZWAVE_GATEWAY_IP')
	print "zwave gateway IP: "+ZWAVE_GATEWAY_IP
