/*
  TestWKPF.java
 */

import java.io.*;
import nanovm.wkpf.*;

public class TestWKPF {
    private static void callVirtualProfileUpdates() {
        VirtualProfile profile;
        while ((profile=WKPF.select())!=null)
            profile.update();
    }
    
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

        System.out.println("setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): operator=>, threshold=1000, value=1200");
        WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_ID_THRESHOLD_OPERATOR, VirtualThresholdProfile.THRESHOLD_PROFILE_OPERATOR_GT);
        System.out.println("======="+WKPF.getErrorCode());
        WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_ID_THRESHOLD_THRESHOLD, (short)1000);
        System.out.println("======="+WKPF.getErrorCode());
        WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_ID_THRESHOLD_VALUE, (short)1200);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Calling update() on profile returned by WKPF.select() (the threshold profile instance should be returned)");
        callVirtualProfileUpdates();

        System.out.print("Getting output of threshold profile:");
        if(WKPF.getPropertyBoolean(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_OUTPUT))
            System.out.println("TRUE");
        else
            System.out.println("FALSE");
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("setting value to 800");
        WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_ID_THRESHOLD_VALUE, (short)800);
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Calling update() on profile returned by WKPF.select() (the threshold profile instance should be returned)");
        callVirtualProfileUpdates();

        System.out.print("Getting output of threshold profile:");
        if(WKPF.getPropertyBoolean(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_OUTPUT))
            System.out.println("TRUE");
        else
            System.out.println("FALSE");
        System.out.println("======="+WKPF.getErrorCode());

        System.out.println("Calling WKPF.select() again. Should return null now");
        VirtualProfile nullProfile = WKPF.select();
        if (nullProfile == null)
            System.out.println("OK");
        else
            System.out.println("FAILED!");

        System.out.println("Setting value using internal setPropertyShort method (the one the profiles should use internally)");
        WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_VALUE, (short)123);

        System.out.println("Calling WKPF.select() again. It should still return null");
        nullProfile = WKPF.select();
        if (nullProfile == null)
            System.out.println("OK");
        else
            System.out.println("FAILED!");

        System.out.println("Clearing dirty properties");
        int dirtyProperty = 0;
        while((dirtyProperty = WKPF.getNextDirtyProperty()) != 0) { }
        
        System.out.println("Setting value (property " + WKPF.PROPERTY_ID_THRESHOLD_VALUE + ") using external setPropertyShort method");
        WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_ID_THRESHOLD_VALUE, (short)100);
        System.out.println("Setting threshold (property " + WKPF.PROPERTY_ID_THRESHOLD_THRESHOLD + ") using internal setPropertyShort method (the one the profiles should use internally)");
        WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_ID_THRESHOLD_THRESHOLD, (short)200);
        System.out.println("Both should be returned by WKPF.getNextDirtyProperty()");        
        while((dirtyProperty = WKPF.getNextDirtyProperty()) != 0) {
            int portNumber = (short)(dirtyProperty >> 8) & (short)0xFF;
            int propertyNumber = dirtyProperty & (short)0xFF;
            System.out.println("port: " + portNumber + " property: " + propertyNumber);
        }
        System.out.println("Done");

        System.out.println("WuKong Profile Framework test - done");
    }
}
