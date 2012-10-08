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
    WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, (byte)0x20, wuclassInstanceThreshold);
    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OPERATOR, WKPF.ENUM_THRESHOLD_OPERATOR_GT);
    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)1000);
    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);

    // Temperature Sensor is not implemented yet
    //WKPF.createWuObject((short)WKPF.WUCLASS_TEMPERATURE_SENSOR, (byte)0x20, null);

    WKPF.createWuObject((short)WKPF.WUCLASS_LIGHT_SENSOR, (byte)0x23, null);
    WKPF.setPropertyRefreshRate(WKPF.WUCLASS_LIGHT_SENSOR, WKPF.PROPERTY_LIGHT_SENSOR_REFRESH_RATE, (short)12);

    WKPF.createWuObject((short)WKPF.WUCLASS_LIGHT_ACTUATOR, (byte)0x24, null);
    WKPF.setPropertyBoolean(WKPF.WUCLASS_LIGHT_ACTUATOR, WKPF.PROPERTY_LIGHT_ACTUATOR_ON_OFF, true);

    WKPF.createWuObject((short)WKPF.WUCLASS_NUMERIC_CONTROLLER, (byte)0x26, null);
    WKPF.setPropertyShort(WKPF.WUCLASS_NUMERIC_CONTROLLER, WKPF.PROPERTY_NUMERIC_CONTROLLER_OUTPUT, (short)25);

    while (true) {} // Need loop to prevent it from exiting the program
  }
}
