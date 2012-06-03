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
/*
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
*/
    VirtualWuObject wuclassInstanceThreshold = new VirtualThresholdWuObject();

    WKPF.registerWuClass(WKPF.WUCLASS_THRESHOLD, GENERATEDVirtualThresholdWuObject.properties);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualThresholdWuObject.");


    WKPF.createWuObject((short)WKPF.WUCLASS_THRESHOLD, (byte)0x20, wuclassInstanceThreshold);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual Threshold wuclass at port 0x20.");

    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OPERATOR, WKPF.ENUM_THRESHOLD_OPERATOR_GT);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): operator=>");


    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_THRESHOLD, (short)1000);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): threshold=1000");
    WKPF.setPropertyShort(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_VALUE, (short)1200);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "setup initial properties using methods that will be called from propertyDispatch (to cause update() to be triggered): value=1200");
    
    wuclassInstanceThreshold.update();

    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceThreshold, WKPF.PROPERTY_THRESHOLD_OUTPUT), true, "Getting output of virtual threshold wuclass, should be true.");

    //condition selector test
    /*VirtualWuObject wuclassInstanceConditionSelectorBoolean = new VirtualConditionSelectorBooleanWuObject();
    WKPF.registerWuClass(WKPF.WUCLASS_CONDITION_SELECTOR_BOOLEAN, GENERATEDVirtualConditionSelectorBooleanWuObject.properties);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualConditionSelectorBooleanWuObject.");
    WKPF.createWuObject((short)WKPF.WUCLASS_CONDITION_SELECTOR_BOOLEAN, (byte)0x23, wuclassInstanceConditionSelectorBoolean);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual condition selector boolean wuclass at port 0x23.");
    WKPF.setPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_INPUT, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input = true");
    WKPF.setPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_CONTROL, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "control = false");
    wuclassInstanceConditionSelectorBoolean.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT1), true, "output1 should be true.");
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT2), false, "output2 should be false.");
    WKPF.setPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_INPUT, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input = true");
    WKPF.setPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_CONTROL, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "control = true");
    wuclassInstanceConditionSelectorBoolean.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT1), false, "output1 should be false.");
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceConditionSelectorBoolean, WKPF.PROPERTY_CONDITION_SELECTOR_BOOLEAN_OUTPUT2), true, "output2 should be true.");

    VirtualWuObject wuclassInstanceConditionSelectorShort = new VirtualConditionSelectorShortWuObject();
    WKPF.registerWuClass(WKPF.WUCLASS_CONDITION_SELECTOR_SHORT, GENERATEDVirtualConditionSelectorShortWuObject.properties);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualConditionSelectorShortWuObject.");
    WKPF.createWuObject((short)WKPF.WUCLASS_CONDITION_SELECTOR_SHORT, (byte)0x24, wuclassInstanceConditionSelectorShort);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual condition selector short wuclass at port 0x24.");
    WKPF.setPropertyShort(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_INPUT, (short)60);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input = true");
    WKPF.setPropertyBoolean(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_CONTROL, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "control = false");
    wuclassInstanceConditionSelectorShort.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT1), 60, "output1 should be 60.");
    assertEqual(WKPF.getPropertyShort(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT2), 0, "output2 should be 0.");
    WKPF.setPropertyShort(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_INPUT, (short)70);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input = 70");
    WKPF.setPropertyBoolean(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_CONTROL, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "control = true");
    wuclassInstanceConditionSelectorShort.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT1), 0, "output1 should be 0.");
    assertEqual(WKPF.getPropertyShort(wuclassInstanceConditionSelectorShort, WKPF.PROPERTY_CONDITION_SELECTOR_SHORT_OUTPUT2), 70, "output2 should be 70.");*/


    //MATH TEST
    /*VirtualWuObject wuclassInstanceMath = new VirtualMathWuObject();
    WKPF.registerWuClass(WKPF.WUCLASS_MATH, GENERATEDVirtualMathWuObject.properties);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualMathWuObject.");
    WKPF.createWuObject((short)WKPF.WUCLASS_MATH, (byte)0x22, wuclassInstanceMath);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual math wuclass at port 0x22.");

    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_MAX);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = MAX");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_INPUT1, (short)40);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input1 = 40");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_INPUT2, (short)20);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input1 = 20");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_INPUT3, (short)10);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input1 = 10");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_INPUT4, (short)30);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input1 = 30");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)40, "output should be 40.");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_MIN);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = MIN");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)10, "output should be 10.");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_AVG);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = AVG");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)25, "output should be 25.");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_ADD);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = ADD");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)100, "output should be 100.");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_SUB);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = SUB");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)20, "output should be 20.");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_DIVIDE);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = DIVIDE");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)2, "output should be 2.");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_INPUT3, (short)1);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input1 = 1");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_INPUT4, (short)3);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "input1 = 3");
    WKPF.setPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OPERATOR, WKPF.ENUM_MATH_OPERATOR_MULTIPLY);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = MULTIPLY");
    wuclassInstanceMath.update();
    assertEqual(WKPF.getPropertyShort(wuclassInstanceMath, WKPF.PROPERTY_MATH_OUTPUT), (short)2400, "output should be 2400.");
*/

    //LOGICAL TEST
    /*VirtualWuObject wuclassInstanceLogical = new VirtualLogicalWuObject();
    WKPF.registerWuClass(WKPF.WUCLASS_LOGICAL, GENERATEDVirtualLogicalWuObject.properties);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Registering VirtualLogicalWuObject.");
    WKPF.createWuObject((short)WKPF.WUCLASS_LOGICAL, (byte)0x21, wuclassInstanceLogical);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "Creating wuobject for virtual logical wuclass at port 0x21.");

    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_AND);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = AND");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT1, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT1 = false");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT2, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT2 = false");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT3, false);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT3 = false");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT4, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT4 = true");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), false, "output should be false.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_OR);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = OR");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), true, "output should be true.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_NOT);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = NOT");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), true, "output should be true.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_XOR);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = XOR");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), true, "output should be true.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_AND);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = AND");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT1, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT1 = true");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT2, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT2 = true");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT3, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT3 = true");
    WKPF.setPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_INPUT4, true);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "INPUT4 = true");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), true, "output should be true.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_OR);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = OR");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), true, "output should be true.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_NOT);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = NOT");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), false, "output should be false.");
    WKPF.setPropertyShort(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OPERATOR, WKPF.ENUM_LOGICAL_OPERATOR_XOR);
    assertEqual(WKPF.getErrorCode(), WKPF.OK, "operator = XOR");
    wuclassInstanceLogical.update();
    assertEqualBoolean(WKPF.getPropertyBoolean(wuclassInstanceLogical, WKPF.PROPERTY_LOGICAL_OUTPUT), false, "output should be false.");*/

    System.out.println("WuKong WuClass Framework test - done. Passed:" + passedCount + " Failed:" + failedCount);
  }
}
