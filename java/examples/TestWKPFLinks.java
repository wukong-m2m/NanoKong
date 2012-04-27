/*
TestWKPF.java
*/

import java.io.*;
import nanovm.wkpf.*;

public class TestWKPFLinks {
  private static int passedCount=0;
  private static int failedCount=0;

  private static void callVirtualProfileUpdates() {
    VirtualProfile profile;
    while ((profile=WKPF.select())!=null)
        profile.update();
  }

  public static void assertEqual(int value, int expected, String message) {
    if (value == expected) {
      System.out.println("OK: " + message);
      passedCount++;
    } else {
      System.out.println("----------->FAIL: " + message);
      System.out.println("Expected: " + expected + " Got: " + value);
      failedCount++;
    }
  }
  public static void assertEqualBoolean(boolean value, boolean expected, String message) {
    if (value == expected) {
      System.out.println("OK: " + message);
      passedCount++;
    } else {
      System.out.println("----------->FAIL: " + message);
      failedCount++;
    }
  }
  public static void assertEqualObject(Object value, Object expected, String message) {
    if (value == expected) {
      System.out.println("OK: " + message);
      passedCount++;
    } else {
      System.out.println("----------->FAIL: " + message);
      failedCount++;
    }
  }
  
  private static class VirtualTestProfile extends VirtualProfile {
    private String name;
    
    public VirtualTestProfile(String name) {
      this.name = name;
    }
    
    public static final byte[] properties = new byte[] {
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
      WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW,
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
      };

    public void update() {
      System.out.println("");
      System.out.println("UPDATE ON VIRTUAL PROFILE " + name);
      System.out.println("PROPERTY 0: " + WKPF.getPropertyShort(this, (byte)0));
      if (WKPF.getPropertyBoolean(this, (byte)1))
        System.out.println("PROPERTY 1: TRUE");
      else
        System.out.println("PROPERTY 1: FALSE");
      System.out.println("PROPERTY 2: " + WKPF.getPropertyShort(this, (byte)2));
      System.out.println("");
    }
  }

  public static void main(String[] args) {
    System.out.println("WuKong Profile Framework test");

    byte myNodeId = (byte)WKPF.getMyNodeId();

    WKPF.registerProfile((short)0x42, VirtualTestProfile.properties);
    VirtualProfile profileInstanceA = new VirtualTestProfile("A");
    WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceA);
    VirtualProfile profileInstanceB = new VirtualTestProfile("B");
    WKPF.createEndpoint((short)0x42, (byte)0x20, profileInstanceB);

    int COMPONENT_INSTANCE_ID_A = 0;
    int COMPONENT_INSTANCE_ID_B = 1;

    byte[] componentInstanceToEndpointMap = { // Indexed by component instance id.
      myNodeId, (byte)0x10, // Component 0: instance A        @ node 1, port 1
      myNodeId, (byte)0x20  // Component 1: instance B        @ node 1, port 2
    };
    
    byte[] linkDefinitions = {
      // Connect instance A, property 0 to instance B property 2 (short)
      (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)0,
      (byte)COMPONENT_INSTANCE_ID_B, (byte)0, (byte)2,
      (byte)0x42, (byte)0,

      // Connect instance A, property 0 to instance B property 0 (short)
      (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)0,
      (byte)COMPONENT_INSTANCE_ID_B, (byte)0, (byte)0,
      (byte)0x42, (byte)0,

      // Connect instance B, property 1 to instance A property 1 (boolean)
      (byte)COMPONENT_INSTANCE_ID_B, (byte)0, (byte)1,
      (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)1,
      (byte)0x42, (byte)0
    };
    
    WKPF.loadComponentToEndpointMap(componentInstanceToEndpointMap);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering component to endpoint map.");
    
    WKPF.loadLinkDefinitions(linkDefinitions);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering link definitions.");
    
    WKPF.setPropertyShort(profileInstanceA, (byte)0, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 on instance A to 123.");
    callVirtualProfileUpdates();

    System.out.println("WuKong Profile Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
  }
}
