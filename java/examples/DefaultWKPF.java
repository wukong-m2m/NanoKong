/*
DefaultWKPF.java
*/

import java.io.*;
import nanovm.wkpf.*;

public class DefaultWKPF {
  private static int passedCount=0;
  private static int failedCount=0;

  private static class VirtualTestWuClass extends VirtualWuObject {
    public static final byte[] properties = new byte[] {
      WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_READWRITE,
      WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READWRITE};

    public void update() {}
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
    System.out.println("WuKong Default Wuclass java");

    VirtualWuObject wuclassInstanceThreshold = new VirtualThresholdWuObject();
    WKPF.registerWuClass(WKPF.WUCLASS_THRESHOLD, GENERATEDVirtualThresholdWuObject.properties);
    WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, (byte)0x00, wuclassInstanceThreshold);

    WKPF.createWuObject((short)WKPF.WUCLASS_TEMPERATURE_SENSOR, (byte)0x20, null);

    WKPF.createWuObject((short)WKPF.WUCLASS_LIGHT_SENSOR, (byte)0x23, null);

    WKPF.createWuObject((short)WKPF.WUCLASS_LIGHT_ACTUATOR, (byte)0x24, null);

    WKPF.createWuObject((short)WKPF.WUCLASS_NUMERIC_CONTROLLER, (byte)0x26, null);

    while (true) {} // Need loop to prevent it from exiting the program
  }
}
