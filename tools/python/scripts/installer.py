#!/usr/bin/env python
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../master'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../master/wkpf'))
print os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../master/wkpf')

from wkpf.pynvc import *
from wkpf.wkpfcomm import *

comm = getComm()

print "node ids", comm.getNodeIds()

comm.setFeature(2, WKPF_FEATURE_LIGHT_SENSOR, 0)
comm.setFeature(2, WKPF_FEATURE_LIGHT_ACTUATOR, 1)
comm.setFeature(2, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
comm.setFeature(2, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
comm.setLocation(2, "WuKong")

comm.setFeature(7, WKPF_FEATURE_LIGHT_SENSOR, 1)
comm.setFeature(7, WKPF_FEATURE_LIGHT_ACTUATOR, 1)
comm.setFeature(7, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
comm.setFeature(7, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
comm.setLocation(7, "WuKong")

comm.setFeature(4, WKPF_FEATURE_LIGHT_SENSOR, 1)
comm.setFeature(4, WKPF_FEATURE_LIGHT_ACTUATOR, 0)
comm.setFeature(4, WKPF_FEATURE_NUMERIC_CONTROLLER, 1)
comm.setFeature(4, WKPF_FEATURE_NATIVE_THRESHOLD, 1)
comm.setLocation(4, "WuKong")

comm.setFeature(5, WKPF_FEATURE_LIGHT_SENSOR, 0)
comm.setFeature(5, WKPF_FEATURE_LIGHT_ACTUATOR, 1)
comm.setFeature(5, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
comm.setFeature(5, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
comm.setLocation(5, "WuKong")

comm.setFeature(6, WKPF_FEATURE_LIGHT_SENSOR, 0)
comm.setFeature(6, WKPF_FEATURE_LIGHT_ACTUATOR, 1)
comm.setFeature(6, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
comm.setFeature(6, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
comm.setLocation(6, "WuKong")
