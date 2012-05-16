//========== WuClass Definitions ==========
// Generic {'soft': False, 'vrtl': False, 'id': 0, 'prop': {u'dummy': {'access': 'ro', 'type': u'short', 'id': 0}}}
// Threshold {'soft': True, 'vrtl': True, 'id': 1, 'prop': {u'operator': {'access': 'rw', 'type': {u'LT': 1, u'GT': 2, u'GTE': 4, u'LTE': 3}, 'id': 0}, u'threshold': {'access': 'rw', 'type': u'short', 'id': 1}, u'value': {'access': 'rw', 'type': u'short', 'id': 2}, u'output': {'access': 'ro', 'type': u'short', 'id': 3}}}
// Temperature_Sensor {'soft': False, 'vrtl': False, 'id': 2, 'prop': {u'current_temperature': {'access': 'ro', 'type': u'short', 'id': 0}}}
// Logical {'soft': True, 'vrtl': True, 'id': 11, 'prop': {u'input2': {'access': 'rw', 'type': u'boolean', 'id': 1}, u'input3': {'access': 'rw', 'type': u'boolean', 'id': 2}, u'input1': {'access': 'rw', 'type': u'boolean', 'id': 0}, u'input4': {'access': 'rw', 'type': u'boolean', 'id': 3}, u'operator': {'access': 'rw', 'type': {u'AND': 1, u'NOT': 3, u'XOR': 4, u'OR': 2}, 'id': 4}, u'output': {'access': 'ro', 'type': u'boolean', 'id': 5}}}
// Condition_selector_boolean {'soft': True, 'vrtl': True, 'id': 21, 'prop': {u'control': {'access': 'rw', 'type': u'boolean', 'id': 1}, u'input': {'access': 'rw', 'type': u'boolean', 'id': 0}, u'output1': {'access': 'ro', 'type': u'boolean', 'id': 2}, u'output2': {'access': 'ro', 'type': u'boolean', 'id': 3}}}
// Condition_selector_short {'soft': True, 'vrtl': True, 'id': 22, 'prop': {u'control': {'access': 'rw', 'type': u'boolean', 'id': 1}, u'input': {'access': 'rw', 'type': u'short', 'id': 0}, u'output1': {'access': 'ro', 'type': u'short', 'id': 2}, u'output2': {'access': 'ro', 'type': u'short', 'id': 3}}}
// Loop_delay_boolean {'soft': True, 'vrtl': True, 'id': 31, 'prop': {u'delay': {'access': 'rw', 'type': u'short', 'id': 1}, u'input': {'access': 'rw', 'type': u'boolean', 'id': 0}, u'output': {'access': 'ro', 'type': u'boolean', 'id': 2}}}
// Loop_delay_short {'soft': True, 'vrtl': True, 'id': 32, 'prop': {u'delay': {'access': 'rw', 'type': u'short', 'id': 1}, u'input': {'access': 'rw', 'type': u'short', 'id': 0}, u'output': {'access': 'ro', 'type': u'short', 'id': 2}}}
// Math {'soft': True, 'vrtl': True, 'id': 41, 'prop': {u'input2': {'access': 'rw', 'type': u'short', 'id': 1}, u'input3': {'access': 'rw', 'type': u'short', 'id': 2}, u'input1': {'access': 'rw', 'type': u'short', 'id': 0}, u'input4': {'access': 'rw', 'type': u'short', 'id': 3}, u'operator': {'access': 'rw', 'type': {u'SUB': 5, u'MIN': 2, u'MAX': 1, u'ADD': 4, u'MULTIPLY': 6, u'AVG': 3, u'DIVIDE': 7}, 'id': 4}, u'output': {'access': 'ro', 'type': u'short', 'id': 5}, u'remainder': {'access': 'ro', 'type': u'short', 'id': 6}}}
// Numeric_Controller {'soft': False, 'vrtl': False, 'id': 3, 'prop': {u'output': {'access': 'ro', 'type': u'short', 'id': 0}}}
// Light_Sensor {'soft': False, 'vrtl': False, 'id': 5, 'prop': {u'current_value': {'access': 'ro', 'type': u'short', 'id': 0}, u'refresh_rate': {'access': 'rw', 'type': u'short', 'id': 1}}}
// Light {'soft': False, 'vrtl': False, 'id': 4, 'prop': {u'on_Off': {'access': 'rw', 'type': u'boolean', 'id': 0}}}
// Occupancy_Sensor {'soft': True, 'vrtl': True, 'id': 4101, 'prop': {u'occupied': {'access': 'ro', 'type': u'boolean', 'id': 0}}}
// And_Gate {'soft': True, 'vrtl': True, 'id': 6, 'prop': {u'input2': {'access': 'rw', 'type': u'boolean', 'id': 1}, u'output': {'access': 'ro', 'type': u'boolean', 'id': 2}, u'input1': {'access': 'rw', 'type': u'boolean', 'id': 0}}}
//
//========== Components Definitions ==========
// InputController1 {'classid': 3, 'defaults': [(u'output', 127)], 'class': u'Numeric_Controller', 'cmpid': 0}
// LightSensor1 {'classid': 5, 'defaults': [(u'refresh_rate', 5000)], 'class': u'Light_Sensor', 'cmpid': 1}
// Threshold1 {'classid': 1, 'defaults': [(u'operator', 3)], 'class': u'Threshold', 'cmpid': 2}
// Occupancy1 {'classid': 4101, 'defaults': [(u'occupied', True)], 'class': u'Occupancy_Sensor', 'cmpid': 3}
// AndGate1 {'classid': 6, 'defaults': [], 'class': u'And_Gate', 'cmpid': 4}
// Light1 {'classid': 4, 'defaults': [], 'class': u'Light', 'cmpid': 5}
//
//========== Links Definitions ==========
// (u'InputController1', u'output', u'Threshold1', u'threshold', u'Threshold1')
// (0, 0) 0 (2, 0) 1 (1, 0)
// (u'LightSensor1', u'current_value', u'Threshold1', u'value', u'Threshold1')
// (1, 0) 0 (2, 0) 2 (1, 0)
// (u'Threshold1', u'output', u'AndGate1', u'input1', u'AndGate1')
// (2, 0) 3 (4, 0) 0 (6, 0)
// (u'Occupancy1', u'occupied', u'AndGate1', u'input2', u'AndGate1')
// (3, 0) 0 (4, 0) 1 (6, 0)
// (u'AndGate1', u'output', u'Light1', u'on_Off', u'Light1')
// (4, 0) 2 (5, 0) 0 (4, 0)
//
//========== Code ==========
import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;

public class HAScenario2 {
    public static void main (String[] args) {
        System.out.println("app");
        WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
        WKPF.loadLinkDefinitions(linkDefinitions);
        initialiseLocalWuObjects();
        
        while(true){
            VirtualWuObject wuclass = WKPF.select();
            if (wuclass != null) {
                wuclass.update();
            }
        }
    }

    private final static byte[] linkDefinitions = {
        (byte)0, (byte)0, (byte)0,
        (byte)2, (byte)0, (byte)1,
        (byte)1, (byte)0,
        (byte)1, (byte)0, (byte)0,
        (byte)2, (byte)0, (byte)2,
        (byte)1, (byte)0,
        (byte)2, (byte)0, (byte)3,
        (byte)4, (byte)0, (byte)0,
        (byte)6, (byte)0,
        (byte)3, (byte)0, (byte)0,
        (byte)4, (byte)0, (byte)1,
        (byte)6, (byte)0,
        (byte)4, (byte)0, (byte)2,
        (byte)5, (byte)0, (byte)0,
        (byte)4, (byte)0,
    };

    private final static byte[] componentInstanceToWuObjectAddrMap = {
        (byte)1, (byte)0x1, // Component 0    @ node 1, port 1
        (byte)1, (byte)0x2, // Component 1    @ node 1, port 2
        (byte)3, (byte)0x3, // Component 2    @ node 3, port 3
        (byte)3, (byte)0x4, // Component 3    @ node 3, port 4
        (byte)1, (byte)0x5, // Component 4    @ node 1, port 5
        (byte)3, (byte)0x5  // Component 5    @ node 3, port 5
    };

    private static void initialiseLocalWuObjects() {
        WKPF.registerWuClass(WKPF.WUCLASS_THRESHOLD, GENERATEDVirtualThresholdWuObject.properties);
        WKPF.registerWuClass(WKPF.WUCLASS_OCCUPANCY_SENSOR, GENERATEDVirtualOccupancySensorWuObject.properties);
        WKPF.registerWuClass(WKPF.WUCLASS_AND_GATE, GENERATEDVirtualAndGateWuObject.properties);

        if (WKPF.isLocalComponent((short)0)) {
            WKPF.setPropertyShort((short)0, WKPF.PROPERTY_NUMERIC_CONTROLLER_OUTPUT, (short)127);
        }
        if (WKPF.isLocalComponent((short)1)) {
            WKPF.setPropertyRefreshRate((short)1, WKPF.PROPERTY_LIGHT_SENSOR_REFRESH_RATE, (short)5000);
        }
        if (WKPF.isLocalComponent((short)2)) {
            VirtualWuObject wuclassInstanceThreshold = new VirtualThresholdWuObject();
            WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, WKPF.getPortNumberForComponent((short)2), wuclassInstanceThreshold);
            WKPF.setPropertyShort((short)2, WKPF.PROPERTY_THRESHOLD_OPERATOR, (short)3);
        }
        if (WKPF.isLocalComponent((short)3)) {
            VirtualWuObject wuclassInstanceOccupancy_Sensor = new VirtualOccupancySensorWuObject();
            WKPF.createWuObject((short)WKPF.WUCLASS_OCCUPANCY_SENSOR, WKPF.getPortNumberForComponent((short)3), wuclassInstanceOccupancy_Sensor);
            WKPF.setPropertyBoolean((short)3, WKPF.PROPERTY_OCCUPANCY_SENSOR_OCCUPIED, true);
        }
        if (WKPF.isLocalComponent((short)4)) {
            VirtualWuObject wuclassInstanceAnd_Gate = new VirtualAndGateWuObject();
            WKPF.createWuObject((short)WKPF.WUCLASS_AND_GATE, WKPF.getPortNumberForComponent((short)4), wuclassInstanceAnd_Gate);
        }
        if (WKPF.isLocalComponent((short)5)) {
        }
    }
}
    
