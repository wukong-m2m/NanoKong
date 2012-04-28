/*
TestWKPF.java
*/

import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;

public class TestWKPFLinksRemote {
  private static int passedCount=0;
  private static int failedCount=0;
  private static int myNodeId;

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

  private static byte getPortNumberForComponent(int componentId) {
    return componentInstanceToEndpointMap[componentId*2 + 1];
  }
  private static boolean isLocalComponent(int componentId) {    
    int nodeId=componentInstanceToEndpointMap[componentId*2];
    return nodeId == myNodeId;
  }

  private final static int COMPONENT_INSTANCE_ID_A = 0;
  private final static int COMPONENT_INSTANCE_ID_B = 1;
  private final static int COMPONENT_INSTANCE_ID_C = 2;

  private final static byte[] componentInstanceToEndpointMap = { // Indexed by component instance id.
    (byte)1, (byte)0x10, // Component 0: instance A        @ node 1, port 0x10
    (byte)3, (byte)0x20, // Component 1: instance B        @ node 3, port 0x20
    (byte)1, (byte)0x20  // Component 2: instance C        @ node 1, port 0x20
  };
  
  private final static byte[] linkDefinitions = { // Component instance id and profile id are little endian
    // Connect instance A, property 0 to instance B property 2 (short)
    (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)0,
    (byte)COMPONENT_INSTANCE_ID_B, (byte)0, (byte)2,
    (byte)0x42, (byte)0,

    // Connect instance A, property 0 to instance B property 0 (short)
    (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)0,
    (byte)COMPONENT_INSTANCE_ID_B, (byte)0, (byte)0,
    (byte)0x42, (byte)0,

    // Connect instance A, property 0 to instance C property 2 (short)
    (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)0,
    (byte)COMPONENT_INSTANCE_ID_C, (byte)0, (byte)2,
    (byte)0x42, (byte)0,

    // Connect instance A, property 1 to instance B property 1 (boolean)
    (byte)COMPONENT_INSTANCE_ID_A, (byte)0, (byte)1,
    (byte)COMPONENT_INSTANCE_ID_B, (byte)0, (byte)1,
    (byte)0x42, (byte)0
  };

  public static void main(String[] args) {    
    System.out.println("WuKong Profile Framework Link test");

    myNodeId = WKPF.getMyNodeId();
    System.out.println("My node id: " + myNodeId);

    WKPF.registerProfile((short)0x42, VirtualTestProfile.properties);
    VirtualProfile profileInstanceA = null, profileInstanceB = null, profileInstanceC = null;
    if (isLocalComponent(COMPONENT_INSTANCE_ID_A)) {
      profileInstanceA = new VirtualTestProfile("A");
      WKPF.createEndpoint((short)0x42, getPortNumberForComponent(COMPONENT_INSTANCE_ID_A), profileInstanceA);
    }
    if (isLocalComponent(COMPONENT_INSTANCE_ID_B)) {
      profileInstanceB = new VirtualTestProfile("B");
      WKPF.createEndpoint((short)0x42, getPortNumberForComponent(COMPONENT_INSTANCE_ID_B), profileInstanceB);
    }
    if (isLocalComponent(COMPONENT_INSTANCE_ID_C)) {
      profileInstanceC = new VirtualTestProfile("C");
      WKPF.createEndpoint((short)0x42, getPortNumberForComponent(COMPONENT_INSTANCE_ID_C), profileInstanceC);
    }

    WKPF.loadComponentToEndpointMap(componentInstanceToEndpointMap);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering component to endpoint map.");
    
    WKPF.loadLinkDefinitions(linkDefinitions);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering link definitions.");

    if (isLocalComponent(COMPONENT_INSTANCE_ID_A)) {
      WKPF.setPropertyShort(profileInstanceA, (byte)0, (short)123);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 on instance A to 123.");
      callVirtualProfileUpdates();

      WKPF.setPropertyShort(profileInstanceA, (byte)0, (short)124);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 on instance A to 124.");
      callVirtualProfileUpdates();

      WKPF.setPropertyBoolean(profileInstanceA, (byte)1, true);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 on instance A to true.");
      callVirtualProfileUpdates();

      WKPF.setPropertyBoolean(profileInstanceA, (byte)1, false);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 on instance A to false.");
      callVirtualProfileUpdates();
    }
    if (isLocalComponent(COMPONENT_INSTANCE_ID_B)) {
      while(true) {
        callVirtualProfileUpdates();
        Timer.wait(1000);
      }
    }

    System.out.println("WuKong Profile Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
  }
}
