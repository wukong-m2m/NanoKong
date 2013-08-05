import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;

public class addf3242e6617df792decb7f0d349135 {

    // =========== Begin: Generated by the translator from application WuML
    /* Component instance IDs to indexes:
    Threshold => 0
    Trigger => 1
    Light_Sensor => 2
    Light_Actuator => 3
    */

    //link table
    // fromInstanceIndex(2 bytes), fromPropertyId(1 byte), toInstanceIndex(2 bytes), toPropertyId(1 byte), toWuClassId(2 bytes)
    //eg. (byte)0,(byte)0, (byte)0, (byte)2,(byte)0, (byte)1, (byte)1,(byte)0
    private final static byte[] linkDefinitions = {
        // Note: Component instance id and wuclass id are little endian
        // Note: using WKPF constants now, but this should be generated as literal bytes by the WuML->Java compiler.
        // Connect input controller to threshold
        (byte)0, (byte)0, (byte)3, (byte)1, (byte)0, (byte)0, (byte)10, (byte)0,
        (byte)1, (byte)0, (byte)2, (byte)3, (byte)0, (byte)0, (byte)4, (byte)0,
        (byte)2, (byte)0, (byte)0, (byte)0, (byte)0, (byte)2, (byte)1, (byte)0
    };

    //component node id and port number table
    // each row corresponds to the component index mapped from component ID above
    // each row has two items: node id, port number
    private final static byte[][] componentInstanceToWuObjectAddrMap = {
      new byte[]{
        10, 1
      },
      new byte[]{
        10, 2
      },
      new byte[]{
        10, 3
      },
      new byte[]{
        10, 4
      },
    };

    private final static byte[][] heartbeatToNodeAddrMap = {
      new byte[]{
        10
      },
    };
    // =========== End: Generated by the translator from application WuML

    public static void main (String[] args) {
        System.out.println("addf3242e6617df792decb7f0d349135");
        System.out.println(WKPF.getMyNodeId());
        WKPF.loadHeartbeatToNodeAddrMap(heartbeatToNodeAddrMap);
        WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
        WKPF.loadLinkDefinitions(linkDefinitions);
        initialiseLocalWuObjects();

        while(true){
            VirtualWuObject wuobject = WKPF.select();
            if (wuobject != null) {
                wuobject.update();
            }
        }
    }

    private static void initialiseLocalWuObjects() {
        //all WuClasses from the same group has the same instanceIndex and wuclass
        if (WKPF.isLocalComponent((short)0)) {

        // Native WuClasses (C)
        WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, WKPF.getPortNumberForComponent((short)0), null);WKPF.setPropertyShort((short)0, WKPF.PROPERTY_THRESHOLD_OPERATOR, WKPF.ENUM_THRESHOLD_OPERATOR_LT);WKPF.setPropertyShort((short)0, WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)30);WKPF.setPropertyShort((short)0, WKPF.PROPERTY_THRESHOLD_VALUE, (short)20);WKPF.setPropertyBoolean((short)0, WKPF.PROPERTY_THRESHOLD_OUTPUT, false);}
        //all WuClasses from the same group has the same instanceIndex and wuclass
        if (WKPF.isLocalComponent((short)1)) {

        // Native WuClasses (C)
        WKPF.createWuObject((short)WKPF.WUCLASS_TRIGGER, WKPF.getPortNumberForComponent((short)1), null);WKPF.setPropertyRefreshRate((short)1, WKPF.PROPERTY_TRIGGER_REFRESH_RATE, (short)500);WKPF.setPropertyBoolean((short)1, WKPF.PROPERTY_TRIGGER_OUTPUT, false);}
        //all WuClasses from the same group has the same instanceIndex and wuclass
        if (WKPF.isLocalComponent((short)2)) {

        // Native WuClasses (C)
        WKPF.createWuObject((short)WKPF.WUCLASS_LIGHT_SENSOR, WKPF.getPortNumberForComponent((short)2), null);WKPF.setPropertyRefreshRate((short)2, WKPF.PROPERTY_LIGHT_SENSOR_REFRESH_RATE, (short)250);}
        //all WuClasses from the same group has the same instanceIndex and wuclass
        if (WKPF.isLocalComponent((short)3)) {

        // Native WuClasses (C)
        WKPF.createWuObject((short)WKPF.WUCLASS_LIGHT_ACTUATOR, WKPF.getPortNumberForComponent((short)3), null);WKPF.setPropertyBoolean((short)3, WKPF.PROPERTY_LIGHT_ACTUATOR_ON_OFF, false);}
    }
}