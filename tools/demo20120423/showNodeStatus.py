#!/usr/bin/python
import sys
sys.path.append("/Users/niels/git/nanokong/tools/python")
import wkpf
from wkpf import Endpoint


numericInputEndpoint = Endpoint(nodeId=1, portNumber=1, profileId=3)
lightSensorEndpoint = Endpoint(nodeId=1, portNumber=2, profileId=5)
thresholdEndpointScenario1 = Endpoint(nodeId=1, portNumber=3, profileId=1)
thresholdEndpointScenario2 = Endpoint(nodeId=3, portNumber=3, profileId=1)
occupancyEndpoint = Endpoint(nodeId=1, portNumber=5, profileId=0x1005)
andGateEndpoint = Endpoint(nodeId=3, portNumber=6, profileId=0x1006)
lightEndpoint = Endpoint(nodeId=3, portNumber=4, profileId=4)

endpointsNode1 = wkpf.getEndpointList(1)
endpointsNode3 = wkpf.getEndpointList(3)

profiles = wkpf.getProfileList(3)
if 0x1006 in profiles: # Scenario 2
  light_sensor_value = wkpf.getProperty(lightSensorEndpoint, propertyNumber=0)
  input_value = wkpf.getProperty(numericInputEndpoint, propertyNumber=0)
  threshold_operator = wkpf.getProperty(thresholdEndpointScenario2, propertyNumber=0)
  threshold_threshold = wkpf.getProperty(thresholdEndpointScenario2, propertyNumber=1)
  threshold_value = wkpf.getProperty(thresholdEndpointScenario2, propertyNumber=2)
  threshold_output = wkpf.getProperty(thresholdEndpointScenario2, propertyNumber=3)
  occupancy_value = wkpf.getProperty(occupancyEndpoint, propertyNumber=0)
  andgate_in1 = wkpf.getProperty(andGateEndpoint, propertyNumber=0)
  andgate_in2 = wkpf.getProperty(andGateEndpoint, propertyNumber=1)
  andgate_out = wkpf.getProperty(andGateEndpoint, propertyNumber=2)
  light_value = wkpf.getProperty(lightEndpoint, propertyNumber=0)

  print ""
  print ""
  print "=== Light sensor"
  print "value:", light_sensor_value
  print "=== Input"
  print "value:", input_value
  print "=== Threshold"
  print "operator:", threshold_operator
  print "threshold:", threshold_threshold
  print "value:", threshold_value
  print "output:", threshold_output
  print "=== Occupacy"
  print "value:", occupancy_value
  print "=== And Gate"
  print "in1 (threshold):", andgate_in1
  print "in2 (occupancy):", andgate_in2
  print "value:", andgate_out
  print "=== Light"
  print "value:", light_value
  print "=== Endpoints on node 1"
  print endpointsNode1
  print "=== Endpoints on node 3"
  print endpointsNode3  
else: # Scenario 1
  light_sensor_value = wkpf.getProperty(lightSensorEndpoint, propertyNumber=0)
  input_value = wkpf.getProperty(numericInputEndpoint, propertyNumber=0)
  threshold_operator = wkpf.getProperty(thresholdEndpointScenario1, propertyNumber=0)
  threshold_threshold = wkpf.getProperty(thresholdEndpointScenario1, propertyNumber=1)
  threshold_value = wkpf.getProperty(thresholdEndpointScenario1, propertyNumber=2)
  threshold_output = wkpf.getProperty(thresholdEndpointScenario1, propertyNumber=3)
  light_value = wkpf.getProperty(lightEndpoint, propertyNumber=0)

  print ""
  print ""
  print "=== Light sensor"
  print "value:", light_sensor_value
  print "=== Input"
  print "value:", input_value
  print "=== Threshold"
  print "operator:", threshold_operator
  print "threshold:", threshold_threshold
  print "value:", threshold_value
  print "output:", threshold_output
  print "=== Light"
  print "value:", light_value
  print "=== Endpoints on node 1"
  print endpointsNode1
  print "=== Endpoints on node 3"
  print endpointsNode3

  

