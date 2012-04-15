/*
  TestWKPF.java
 */

import java.io.*;
import nanovm.wkpf.*;

public class TestWKPF {
    public static void main(String[] args) {
        System.out.println("WuKong Profile Framework test");

        System.out.println("Registering VirtualTestProfile");
        WKPF.registerProfile((short)0x42, VirtualTestProfile.properties, (byte)VirtualTestProfile.properties.length);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Creating endpoint for profile instance A at port 0x10");
        VirtualProfile profileInstanceA = new VirtualTestProfile();
        WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceA);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Creating another endpoint for profile instance B at port 0x10, should fail with error " + WKPF.ERR_PORT_IN_USE + "");
        VirtualProfile profileInstanceB = new VirtualTestProfile();
        WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceB);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Removing profile instance A and endpoint at port 0x10");
        WKPF.removeEndpoint((byte)0x10);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Creating another endpoint for profile instance B at port 0x10, now it should work");
        WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceB);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Setting property 0 for profile instance B to 123");
        WKPF.setPropertyShort(profileInstanceB, (byte)0, (short)123);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Getting value for property 0 for profile instance B:" + WKPF.getPropertyShort(profileInstanceB, (byte)0));
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Setting property 0 for profile instance A to 123, should fail with error " + WKPF.ERR_ENDPOINT_NOT_FOUND);
        WKPF.setPropertyShort(profileInstanceA, (byte)0, (short)123);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Setting property 1 for profile instance B to 123, should fail with error " + WKPF.ERR_WRONG_DATATYPE);
        WKPF.setPropertyShort(profileInstanceB, (byte)1, (short)123);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Setting property 1 for profile instance B to true");
        WKPF.setPropertyBoolean(profileInstanceB, (byte)1, true);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.print("Getting value for property 1 for profile instance B:");
        if(WKPF.getPropertyBoolean(profileInstanceB, (byte)1))
            System.out.println("TRUE");
        else
            System.out.println("FALSE");
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Setting property 1 for profile instance B to false");
        WKPF.setPropertyBoolean(profileInstanceB, (byte)1, false);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.print("Getting value for property 1 for profile instance B:");
        if(WKPF.getPropertyBoolean(profileInstanceB, (byte)1))
            System.out.println("TRUE");
        else
            System.out.println("FALSE");
        System.out.println("======="+WKPF.getErrorCode());
        
        System.out.print("Creating endpoint for virtual Threshold profile at port 0x20:");
        VirtualProfile profileInstanceThreshold = new VirtualThresholdProfile();
        WKPF.createEndpoint((short)WKPF.PROFILE_ID_THRESHOLD, (byte)0x20, profileInstanceThreshold);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("setup initial properties: operator=>, threshold=1000, value=800");
        WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_OPERATOR, VirtualThresholdProfile.THRESHOLD_PROFILE_OPERATOR_GT);
        System.out.println("======="+WKPF.getErrorCode());
        WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_THRESHOLD, (short)1000);
        System.out.println("======="+WKPF.getErrorCode());
        WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_VALUE, (short)800);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Calling update() on profile manually (should be done by the framework eventually)");
        profileInstanceThreshold.update();

        System.out.print("Getting output of threshold profile:");
        if(WKPF.getPropertyBoolean(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_OUTPUT))
            System.out.println("TRUE");
        else
            System.out.println("FALSE");
        System.out.println("======="+WKPF.getErrorCode());

        System.out.print("setting value to 1200");
        WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_VALUE, (short)1200);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Calling update() on profile manually (should be done by the framework eventually)");
        profileInstanceThreshold.update();

        System.out.print("Getting output of threshold profile:");
        if(WKPF.getPropertyBoolean(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_OUTPUT))
            System.out.println("TRUE");
        else
            System.out.println("FALSE");
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("WuKong Profile Framework test - done");
    }
}
