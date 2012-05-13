#!/usr/bin/python
import sys
sys.path.append("/Users/niels/git/nanokong/tools/python")
import wkpf
from wkpf import WuObject


numericInputWuObject = WuObject(nodeId=1, portNumber=1, wuclassId=3)
lightSensorWuObject = WuObject(nodeId=1, portNumber=2, wuclassId=5)
thresholdWuObjectScenario1 = WuObject(nodeId=1, portNumber=3, wuclassId=1)
thresholdWuObjectScenario2 = WuObject(nodeId=3, portNumber=3, wuclassId=1)
occupancyWuObject = WuObject(nodeId=1, portNumber=5, wuclassId=0x1005)
andGateWuObject = WuObject(nodeId=3, portNumber=6, wuclassId=0x1006)
lightWuObject = WuObject(nodeId=3, portNumber=4, wuclassId=4)

wuobjectsNode1 = wkpf.getWuObjectList(1)
wuobjectsNode3 = wkpf.getWuObjectList(3)

wuclasses = wkpf.getWuClassList(3)
if 0x1006 in wuclasses: # Scenario 2
  light_sensor_value = wkpf.getProperty(lightSensorWuObject, propertyNumber=0)
  input_value = wkpf.getProperty(numericInputWuObject, propertyNumber=0)
  threshold_operator = wkpf.getProperty(thresholdWuObjectScenario2, propertyNumber=0)
  threshold_threshold = wkpf.getProperty(thresholdWuObjectScenario2, propertyNumber=1)
  threshold_value = wkpf.getProperty(thresholdWuObjectScenario2, propertyNumber=2)
  threshold_output = wkpf.getProperty(thresholdWuObjectScenario2, propertyNumber=3)
  occupancy_value = wkpf.getProperty(occupancyWuObject, propertyNumber=0)
  andgate_in1 = wkpf.getProperty(andGateWuObject, propertyNumber=0)
  andgate_in2 = wkpf.getProperty(andGateWuObject, propertyNumber=1)
  andgate_out = wkpf.getProperty(andGateWuObject, propertyNumber=2)
  light_value = wkpf.getProperty(lightWuObject, propertyNumber=0)

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
  print "=== WuObjects on node 1"
  print wuobjectsNode1
  print "=== WuObjects on node 3"
  print wuobjectsNode3  
else: # Scenario 1
  light_sensor_value = wkpf.getProperty(lightSensorWuObject, propertyNumber=0)
  input_value = wkpf.getProperty(numericInputWuObject, propertyNumber=0)
  threshold_operator = wkpf.getProperty(thresholdWuObjectScenario1, propertyNumber=0)
  threshold_threshold = wkpf.getProperty(thresholdWuObjectScenario1, propertyNumber=1)
  threshold_value = wkpf.getProperty(thresholdWuObjectScenario1, propertyNumber=2)
  threshold_output = wkpf.getProperty(thresholdWuObjectScenario1, propertyNumber=3)
  light_value = wkpf.getProperty(lightWuObject, propertyNumber=0)

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
  print "=== WuObjects on node 1"
  print wuobjectsNode1
  print "=== WuObjects on node 3"
  print wuobjectsNode3

  

