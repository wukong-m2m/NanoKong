import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', 'master')))

from wkpf.pynvc import *
from wkpf.wkpfcomm import *

comm = getComm()

print "node ids", comm.getNodeIds()

#comm.setFeature(2, WKPF_FEATURE_LIGHT_SENSOR, 1)
#comm.setFeature(2, WKPF_FEATURE_LIGHT_ACTUATOR, 0)
#comm.setFeature(2, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
#comm.setFeature(2, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
#comm.setLocation(2, "/Wukong")
comm.setFeature(3, WKPF_FEATURE_LIGHT_SENSOR, 1)
comm.setFeature(3, WKPF_FEATURE_LIGHT_ACTUATOR, 0)
comm.setFeature(3, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
comm.setFeature(3, WKPF_FEATURE_NATIVE_THRESHOLD, 1)
comm.setLocation(3, "/Wukong")
#comm.setFeature(4, WKPF_FEATURE_LIGHT_SENSOR, 0)
#comm.setFeature(4, WKPF_FEATURE_LIGHT_ACTUATOR, 0)
#comm.setFeature(4, WKPF_FEATURE_NUMERIC_CONTROLLER, 1)
#comm.setFeature(4, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
#comm.setLocation(4, "/Wukong")

comm.setFeature(5, WKPF_FEATURE_LIGHT_SENSOR, 0)
comm.setFeature(5, WKPF_FEATURE_LIGHT_ACTUATOR, 1)
comm.setFeature(5, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
comm.setFeature(5, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
comm.setLocation(5, "/Wukong")
#comm.setFeature(10, WKPF_FEATURE_LIGHT_SENSOR, 1)
#comm.setFeature(10, WKPF_FEATURE_TRIGGER, 1)
#comm.setFeature(10, WKPF_FEATURE_LIGHT_ACTUATOR, 1)
#comm.setFeature(10, WKPF_FEATURE_NUMERIC_CONTROLLER, 1)
#comm.setFeature(10, WKPF_FEATURE_NATIVE_THRESHOLD, 1)
#comm.setLocation(10, "/Eng_Hall/Rm4204@(2,1,1)")

#comm.setFeature(14, WKPF_FEATURE_LIGHT_SENSOR, 1)
#comm.setFeature(14, WKPF_FEATURE_LIGHT_ACTUATOR, 0)
#comm.setFeature(14, WKPF_FEATURE_NUMERIC_CONTROLLER, 0)
#comm.setFeature(14, WKPF_FEATURE_NATIVE_THRESHOLD, 0)
#comm.setLocation(14, "/Eng_Hall/Rm4204@(3,1,1)")
