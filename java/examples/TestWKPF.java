/*
TestWKPF.java
*/

import java.io.*;
import nanovm.wkpf.*;

public class TestWKPF {
  private static int passedCount=0;
  private static int failedCount=0;

  private static class VirtualTestProfile extends VirtualProfile {
    public static final byte[] properties = new byte[] {
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
      WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW};

    public void update() {}
  }

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
    System.out.println("WuKong Profile Framework test");

    WKPF.registerProfile((short)0x42, VirtualTestProfile.properties, (byte)VirtualTestProfile.properties.length);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualTestProfile.");

    VirtualProfile profileInstanceA = new VirtualTestProfile();
    WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceA);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating endpoint for profile instance A at port 0x10.");

    VirtualProfile profileInstanceB = new VirtualTestProfile();
    WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceB);
    assertEqual(WKPF.getErrorCode(), WKPF.ERR_PORT_IN_USE, "Creating another endpoint for profile instance B at port 0x10, should fail.");

    WKPF.removeEndpoint((byte)0x10);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Removing profile instance A and endpoint at port 0x10.");

    WKPF.createEndpoint((short)0x42, (byte)0x10, profileInstanceB);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating another endpoint for profile instance B at port 0x10, now it should work.");

    WKPF.setPropertyShort(profileInstanceB, (byte)0, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 0 for profile instance B to 123.");

    assertEqual(WKPF.getPropertyShort(profileInstanceB, (byte)0), 123, "Getting value for property 0 for profile instance B, should be 123.");

    WKPF.setPropertyShort(profileInstanceA, (byte)0, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.ERR_ENDPOINT_NOT_FOUND, "Setting property 0 for profile instance A to 123, should fail.");

    WKPF.setPropertyShort(profileInstanceB, (byte)1, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.ERR_WRONG_DATATYPE, "Setting property 1 for profile instance B to 123, should fail.");

    WKPF.setPropertyBoolean(profileInstanceB, (byte)1, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 for profile instance B to true.");
    assertEqualBoolean(WKPF.getPropertyBoolean(profileInstanceB, (byte)1), true, "Getting value for property 1 for profile instance B, should be true.");

    WKPF.setPropertyBoolean(profileInstanceB, (byte)1, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting property 1 for profile instance B to false.");
    assertEqualBoolean(WKPF.getPropertyBoolean(profileInstanceB, (byte)1), false, "Getting value for property 1 for profile instance B, should be false.");

    VirtualProfile profileInstanceThreshold = new VirtualThresholdProfile();
    WKPF.createEndpoint((short)WKPF.PROFILE_THRESHOLD, (byte)0x20, profileInstanceThreshold);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating endpoint for virtual Threshold profile at port 0x20.");

    WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_THRESHOLD_OPERATOR, VirtualThresholdProfile.THRESHOLD_PROFILE_OPERATOR_GT);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): operator=>");
    WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)1000);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): threshold=1000");
    WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): value=1200");

    callVirtualProfileUpdates();
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Calling update() on profile returned by WKPF.select() (the threshold profile instance should be returned).");
    assertEqualBoolean(WKPF.getPropertyBoolean(profileInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OUTPUT), true, "Getting output of threshold profile, should be true.");

    WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_THRESHOLD_VALUE, (short)800);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting value to 800");
    callVirtualProfileUpdates();
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Calling update() on profile returned by WKPF.select() (the threshold profile instance should be returned).");
    assertEqualBoolean(WKPF.getPropertyBoolean(profileInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OUTPUT), false, "Getting output of threshold profile, should be false.");

    assertEqualObject(WKPF.select(), null, "Calling WKPF.select() again. Should return null now.");

    WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_THRESHOLD_VALUE, (short)123);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting value using internal setPropertyShort method (the one the profiles should use internally).");
    assertEqualObject(WKPF.select(), null, "Calling WKPF.select() again. It should still return null.");

    System.out.println("Clearing dirty properties");
    while(WKPF.loadNextDirtyProperty()) { }

    WKPF.setPropertyShort(profileInstanceThreshold, WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)200);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting threshold (property " + WKPF.PROPERTY_THRESHOLD_THRESHOLD + ") to 200 using internal setPropertyShort method (the one the profiles should use internally)");
    WKPF.setPropertyShort((short)77, (byte)0x20, WKPF.PROPERTY_THRESHOLD_VALUE, (short)500);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Setting value (property " + WKPF.PROPERTY_THRESHOLD_VALUE + ") to 500 using external setPropertyShort method.");
    callVirtualProfileUpdates();
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Calling update() on profile returned by WKPF.select() (the threshold profile instance should be returned).");

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
    
    assertEqual(WKPF.getMyNodeId(), 77, "My node id: 77.");

    System.out.println("WuKong Profile Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
  }
}
