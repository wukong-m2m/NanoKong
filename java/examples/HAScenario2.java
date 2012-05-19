//========== Components Definitions ==========
// 0 InputController1 {'classid': 3, 'defaults': [(u'output', 127)], 'class': u'Numeric_Controller', 'cmpid': 0}
// 1 LightSensor1 {'classid': 5, 'defaults': [(u'refresh_rate', ('r', 1000))], 'class': u'Light_Sensor', 'cmpid': 1}
// 2 Threshold1 {'classid': 1, 'defaults': [(u'operator', u'WKPF.ENUM_THRESHOLD_OPERATOR_LTE')], 'class': u'Threshold', 'cmpid': 2}
// 3 Occupancy1 {'classid': 4101, 'defaults': [(u'occupied', True)], 'class': u'Occupancy_Sensor', 'cmpid': 3}
// 4 AndGate1 {'classid': 6, 'defaults': [], 'class': u'And_Gate', 'cmpid': 4}
// 5 Light1 {'classid': 4, 'defaults': [], 'class': u'Light', 'cmpid': 5}
//
//========== Links Definitions ==========
// fromCompInstanceId(2 bytes), fromPropertyId(1 byte), toCompInstanceId(2 bytes), toPropertyId(1 byte), toWuClassId(2 bytes)
// (u'InputController1', u'output', u'Threshold1', u'threshold', u'Threshold1')
// (0, 0) 0 (2, 0) 1 (1, 0)
// (u'LightSensor1', u'current_value', u'Threshold1', u'value', u'Threshold1')
// (1, 0) 0 (2, 0) 2 (1, 0)
// (u'Threshold1', u'output', u'AndGate1', u'input1', u'AndGate1')
// (2, 0) 3 (4, 0) 0 (6, 0)
// (u'Occupancy1', u'occupied', u'AndGate1', u'input2', u'AndGate1')
// (3, 0) 0 (4, 0) 1 (6, 0)
// (u'AndGate1', u'output', u'Light1', u'on_off', u'Light1')
// (4, 0) 2 (5, 0) 0 (4, 0)
//
//========== Code ==========
import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;

public class HAScenario2 {
    public static void main (String[] args) {
        System.out.println("HAScenario2");
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
        (byte)0,(byte)0, (byte)0,  (byte)2,(byte)0, (byte)1,  (byte)1, (byte)0,
        (byte)1,(byte)0, (byte)0,  (byte)2,(byte)0, (byte)2,  (byte)1, (byte)0,
        (byte)2,(byte)0, (byte)3,  (byte)4,(byte)0, (byte)0,  (byte)6, (byte)0,
        (byte)3,(byte)0, (byte)0,  (byte)4,(byte)0, (byte)1,  (byte)6, (byte)0,
        (byte)4,(byte)0, (byte)2,  (byte)5,(byte)0, (byte)0,  (byte)4, (byte)0,
    };

    private final static byte[] componentInstanceToWuObjectAddrMap = {
        (byte)1, (byte)1, 
        (byte)1, (byte)2, 
        (byte)3, (byte)2, 
        (byte)1, (byte)3, 
        (byte)1, (byte)4, 
        (byte)3, (byte)4, 
    };

    private static void initialiseLocalWuObjects() {
        WKPF.registerWuClass(WKPF.WUCLASS_OCCUPANCY_SENSOR, GENERATEDVirtualOccupancySensorWuObject.properties);
        WKPF.registerWuClass(WKPF.WUCLASS_AND_GATE, GENERATEDVirtualAndGateWuObject.properties);
        if (WKPF.isLocalComponent((short)0)) {
            WKPF.setPropertyShort((short)0, WKPF.PROPERTY_NUMERIC_CONTROLLER_OUTPUT, (short)127);
            System.out.println("=======WKPFERROR" + WKPF.getErrorCode());
        }
        if (WKPF.isLocalComponent((short)1)) {
            WKPF.setPropertyRefreshRate((short)1, WKPF.PROPERTY_LIGHT_SENSOR_REFRESH_RATE, (short)1000);
        }
        if (WKPF.isLocalComponent((short)2)) {
            WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, WKPF.getPortNumberForComponent((short)2), null);
            WKPF.setPropertyShort((short)2, WKPF.PROPERTY_THRESHOLD_OPERATOR, WKPF.ENUM_THRESHOLD_OPERATOR_LTE);
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
        // no need to init component 5
    }
}
    
