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

  private static void callVirtualWuObjectUpdates() {
    VirtualWuObject wuclass;
    while ((wuclass=WKPF.select())!=null)
        wuclass.update();
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
  
  private static class VirtualTestWuClass extends VirtualWuObject {
    private String name;
    
    public VirtualTestWuClass(String name) {
      this.name = name;
    }
    
    public static final byte[] properties = new byte[] {
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
      WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW,
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
      };

    public void update() {
      System.out.println("");
      System.out.println("UPDATE ON VIRTUAL WUCLASS " + name);
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
    return componentInstanceToWuObjectAddrMap[componentId*2 + 1];
  }
  private static boolean isLocalComponent(int componentId) {    
    int nodeId=componentInstanceToWuObjectAddrMap[componentId*2];
    return nodeId == myNodeId;
  }

  private final static int COMPONENT_INSTANCE_ID_A = 0;
  private final static int COMPONENT_INSTANCE_ID_B = 1;
  private final static int COMPONENT_INSTANCE_ID_C = 2;

  private final static byte[] componentInstanceToWuObjectAddrMap = { // Indexed by component instance id.
    (byte)1, (byte)0x10, // Component 0: instance A        @ node 1, port 0x10
    (byte)3, (byte)0x20, // Component 1: instance B        @ node 3, port 0x20
    (byte)1, (byte)0x20  // Component 2: instance C        @ node 1, port 0x20
  };
  
  private final static byte[] linkDefinitions = { // Component instance id and wuclass id are little endian
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
    System.out.println("WuKong WuClass Framework Link test");

    myNodeId = WKPF.getMyNodeId();
    System.out.println("My node id: " + myNodeId);

    WKPF.registerWuClass((short)0x42, VirtualTestWuClass.properties);
    VirtualWuObject wuclassInstanceA = null, wuclassInstanceB = null, wuclassInstanceC = null;
    if (isLocalComponent(COMPONENT_INSTANCE_ID_A)) {
      wuclassInstanceA = new VirtualTestWuClass("A");
      WKPF.createWuObject((short)0x42, getPortNumberForComponent(COMPONENT_INSTANCE_ID_A), wuclassInstanceA);
    }
    if (isLocalComponent(COMPONENT_INSTANCE_ID_B)) {
      wuclassInstanceB = new VirtualTestWuClass("B");
      WKPF.createWuObject((short)0x42, getPortNumberForComponent(COMPONENT_INSTANCE_ID_B), wuclassInstanceB);
    }
    if (isLocalComponent(COMPONENT_INSTANCE_ID_C)) {
      wuclassInstanceC = new VirtualTestWuClass("C");
      WKPF.createWuObject((short)0x42, getPortNumberForComponent(COMPONENT_INSTANCE_ID_C), wuclassInstanceC);
    }

    WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering component to wuobject map.");
    
    WKPF.loadLinkDefinitions(linkDefinitions);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering link definitions.");

    if (isLocalComponent(COMPONENT_INSTANCE_ID_A)) {
      WKPF.setPropertyShort(wuclassInstanceA, (byte)0, (short)123);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 on instance A to 123.");
      callVirtualWuObjectUpdates();

      WKPF.setPropertyShort(wuclassInstanceA, (byte)0, (short)124);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 on instance A to 124.");
      callVirtualWuObjectUpdates();

      WKPF.setPropertyBoolean(wuclassInstanceA, (byte)1, true);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 on instance A to true.");
      callVirtualWuObjectUpdates();

      WKPF.setPropertyBoolean(wuclassInstanceA, (byte)1, false);
      assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 on instance A to false.");
      callVirtualWuObjectUpdates();
    }
    if (isLocalComponent(COMPONENT_INSTANCE_ID_B)) {
      while(true) {
        callVirtualWuObjectUpdates();
        Timer.wait(1000);
      }
    }

    System.out.println("WuKong WuClass Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
  }
}
