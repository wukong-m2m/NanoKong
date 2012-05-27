/*
TestWKPF.java
*/

import java.io.*;
import nanovm.wkpf.*;

public class TestWKPF {
  private static int passedCount=0;
  private static int failedCount=0;

  private static class VirtualTestWuClass extends VirtualWuObject {
    public static final byte[] properties = new byte[] {
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_READWRITE,
      WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READWRITE};

    public void update() {}
  }

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

  public static void main(String[] args) {
    System.out.println("WuKong WuClass Framework test");

    WKPF.registerWuClass((short)0x42, VirtualTestWuClass.properties);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualTestWuClass.");

    VirtualWuObject wuclassInstanceA = new VirtualTestWuClass();
    WKPF.createWuObject((short)0x42, (byte)0x10, wuclassInstanceA);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for wuclass instance A at port 0x10.");

    VirtualWuObject wuclassInstanceB = new VirtualTestWuClass();
    WKPF.createWuObject((short)0x42, (byte)0x10, wuclassInstanceB);
    assertEqual(WKPF.getErrorCode(), WKPF.ERR_PORT_IN_USE, "Creating another wuobject for wuclass instance B at port 0x10, should fail.");

    WKPF.destroyWuObject((byte)0x10);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Removing wuclass instance A and wuobject at port 0x10.");

    WKPF.createWuObject((short)0x42, (byte)0x10, wuclassInstanceB);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating another wuobject for wuclass instance B at port 0x10, now it should work.");

    WKPF.setPropertyShort(wuclassInstanceB, (byte)0, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 for wuclass instance B to 123.");

    assertEqual(WKPF.getPropertyShort(wuclassInstanceB, (byte)0), 123, "Getting value for property 0 for wuclass instance B, should be 123.");

    WKPF.setPropertyShort(wuclassInstanceA, (byte)0, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.ERR_WUOBJECT_NOT_FOUND, "Setting property 0 for wuclass instance A to 123, should fail.");

    WKPF.setPropertyShort(wuclassInstanceB, (byte)1, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.ERR_WRONG_DATATYPE, "Setting property 1 for wuclass instance B to 123, should fail.");

    WKPF.setPropertyBoolean(wuclassInstanceB, (byte)1, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 for wuclass instance B to true.");
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceB, (byte)1), true, "Getting value for property 1 for wuclass instance B, should be true.");

    WKPF.setPropertyBoolean(wuclassInstanceB, (byte)1, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 for wuclass instance B to false.");
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceB, (byte)1), false, "Getting value for property 1 for wuclass instance B, should be false.");

    VirtualWuObject wuclassInstanceThreshold = new VirtualThresholdWuObject();
    WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, (byte)0x20, wuclassInstanceThreshold);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual Threshold wuclass at port 0x20.");

    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_OPERATOR, WKPF.ENUM_THRESHOLD_OPERATOR_GT);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): operator=>");
    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)1000);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): threshold=1000");
    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): value=1200");

    callVirtualWuObjectUpdates();
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Calling update() on wuclass returned by WKPF.select() (the threshold wuclass instance should be returned).");
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OUTPUT), true, "Getting output of virtual threshold wuclass, should be true.");

    WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, (byte)0x30, null);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for native Threshold wuclass at port 0x30.");
    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_OPERATOR, WKPF.ENUM_THRESHOLD_OPERATOR_GT);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): operator=> (compile with DEBUG_WKPFUPDATE to see output)");
    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)1000);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): threshold=1000 (compile with DEBUG_WKPFUPDATE to see output)");
    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): value=1200 (compile with DEBUG_WKPFUPDATE to see output)");

    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_VALUE, (short)800);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting value to 800");
    callVirtualWuObjectUpdates();
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Calling update() on wuclass returned by WKPF.select() (the threshold wuclass instance should be returned).");
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OUTPUT), false, "Getting output of threshold wuclass, should be false.");

    assertEqualObject(WKPF.select(), null, "Calling WKPF.select() again. Should return null now.");

    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_VALUE, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting value using internal setPropertyShort method (the one the wuclasses should use internally).");
    assertEqualObject(WKPF.select(), null, "Calling WKPF.select() again. It should still return null.");

    //System.out.println("Clearing dirty properties");
    //while(WKPF.loadNextDirtyProperty()) { }

    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)200);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting threshold (property " + WKPF.PROPERTY_THRESHOLD_THRESHOLD + ") to 200 using internal setPropertyShort method (the one the wuclasses should use internally)");
    WKPF.setPropertyShort(WKPF.getMyNodeId(), WKPF.PROPERTY_THRESHOLD_VALUE, (short)500);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting value (property " + WKPF.PROPERTY_THRESHOLD_VALUE + ") to 500 using external setPropertyShort method.");
    callVirtualWuObjectUpdates();
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Calling update() on wuclass returned by WKPF.select() (the threshold wuclass instance should be returned).");
/*
    System.out.println("Both should be returned by WKPF.getNextDirtyProperty()");
    assertEqualBoolean(WKPF.loadNextDirtyProperty(), true, "Loading first dirty property.");
    assertEqual(WKPF.getDirtyPropertyPortNumber(), 0x20, "Dirty property: port 0x20.");
    assertEqual(WKPF.getDirtyPropertyNumber(), WKPF.PROPERTY_THRESHOLD_THRESHOLD, "Dirty property: property " + WKPF.PROPERTY_THRESHOLD_THRESHOLD + ".");
    assertEqual(WKPF.getDirtyPropertyShortValue(), 200, "Dirty property: value 200.");
    assertEqualBoolean(WKPF.loadNextDirtyProperty(), true, "Loading second dirty property.");
    assertEqual(WKPF.getDirtyPropertyPortNumber(), 0x20, "Dirty property: port 0x20.");
    assertEqual(WKPF.getDirtyPropertyNumber(), WKPF.PROPERTY_THRESHOLD_VALUE, "Dirty property: property " + WKPF.PROPERTY_THRESHOLD_VALUE + ".");
    assertEqual(WKPF.getDirtyPropertyShortValue(), 500, "Dirty property: value 500.");
    assertEqualBoolean(WKPF.loadNextDirtyProperty(), true, "Loading third dirty property.");
    assertEqual(WKPF.getDirtyPropertyPortNumber(), 0x20, "Dirty property: port 0x20.");
    assertEqual(WKPF.getDirtyPropertyNumber(), WKPF.PROPERTY_THRESHOLD_OUTPUT, "Dirty property: property " + WKPF.PROPERTY_THRESHOLD_OUTPUT + ".");
    assertEqualBoolean(WKPF.getDirtyPropertyBooleanValue(), true, "Dirty property: value true.");
    assertEqualBoolean(WKPF.loadNextDirtyProperty(), false, "No more dirty properties.");
    */
    System.out.println("WuKong WuClass Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
  }
}
