package nanovm.wkpf;

public class WKPF {
  public static native byte getErrorCode(); // Since we can't raise exceptions, I'll use this to signal whether a call succeeds or not.

  // WuClass and wuobject maintenance
  public static native void registerWuClass(short wuclassId, byte[] properties);
  public static native void createWuObject(short wuclassId, byte portNumber, VirtualWuObject virtualWuObjectInstance); // byte or short? No unsigned byte in Java ><
  public static native void destroyWuObject(byte portNumber);

  // Property access for virtual wuclasses
  public static native short getPropertyShort(VirtualWuObject virtualWuObjectInstance, byte propertyNumber);
  public static native void setPropertyShort(VirtualWuObject virtualWuObjectInstance, byte propertyNumber, short value);
  public static native boolean getPropertyBoolean(VirtualWuObject virtualWuObjectInstance, byte propertyNumber);
  public static native void setPropertyBoolean(VirtualWuObject virtualWuObjectInstance, byte propertyNumber, boolean value);

  // Property setters used to initialise properties
  public static native void setPropertyShort(short componentId, byte propertyNumber, short value);
  public static native void setPropertyBoolean(short componentId, byte propertyNumber, boolean value);
  public static native void setPropertyRefreshRate(short componentId, byte propertyNumber, short value);

  // Select function for main loop
  public static native VirtualWuObject select();

  // Load component-wuobject map and link definitions
  public static native void loadComponentToWuObjectAddrMap(byte[] map);
  public static native void loadLinkDefinitions(byte[] links);

  // component-wuobject map related functions
  public static native byte getPortNumberForComponent(short componentId);
  public static native boolean isLocalComponent(short componentId);

  // Who am I?
  public static native short getMyNodeId();

  // Note: need to match definitions in wkpf.h
  public static final byte PROPERTY_TYPE_SHORT                         = 0;
  public static final byte PROPERTY_TYPE_BOOLEAN                       = 1;
  public static final byte PROPERTY_TYPE_REFRESH_RATE                  = 2;
  public static final byte PROPERTY_ACCESS_READ           = (byte)(1 << 7);
  public static final byte PROPERTY_ACCESS_WRITE          = (byte)(1 << 6);
  public static final byte PROPERTY_ACCESS_RW = (PROPERTY_ACCESS_READ|PROPERTY_ACCESS_WRITE);

  public static final byte OK                                             =  0;
  public static final byte ERR_WUOBJECT_NOT_FOUND                         =  1;
  public static final byte ERR_PROPERTY_NOT_FOUND                         =  2;
  public static final byte ERR_WUCLASS_NOT_FOUND                          =  3;
  public static final byte ERR_READ_ONLY                                  =  4;
  public static final byte ERR_WRITE_ONLY                                 =  5;
  public static final byte ERR_PORT_IN_USE                                =  6;
  public static final byte ERR_WUCLASS_ID_IN_USE                          =  7;
  public static final byte ERR_OUT_OF_MEMORY                              =  8;
  public static final byte ERR_WRONG_DATATYPE                             =  9;
  public static final byte ERR_WUOBJECT_ALREADY_ALLOCATED                 = 10;
  public static final byte ERR_NEED_VIRTUAL_WUCLASS_INSTANCE              = 11;
  public static final byte ERR_NVMCOMM_SEND_ERROR                         = 12;
  public static final byte ERR_NVMCOMM_NO_REPLY                           = 13;
  public static final byte ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED = 14;
  public static final byte ERR_SHOULDNT_HAPPEN                    = (byte)0xFF;

  // WuClass library
  public static final short WUCLASS_GENERIC                                    = 0;
  public static final byte PROPERTY_GENERIC_DUMMY                              = 0;

  public static final short WUCLASS_THRESHOLD                                  = 1;
  public static final byte PROPERTY_THRESHOLD_OPERATOR                         = 0;
  public static final byte PROPERTY_THRESHOLD_THRESHOLD                        = 1;
  public static final byte PROPERTY_THRESHOLD_VALUE                            = 2;
  public static final byte PROPERTY_THRESHOLD_OUTPUT                           = 3;
  public static final short PROPERTY_THRESHOLD_OPERATOR_GT                     = 0;
  public static final short PROPERTY_THRESHOLD_OPERATOR_LT                     = 1;
  public static final short PROPERTY_THRESHOLD_OPERATOR_GTE                    = 2;
  public static final short PROPERTY_THRESHOLD_OPERATOR_LTE                    = 3;

  public static final short WUCLASS_TEMPERATURE_SENSOR                         = 2;
  public static final byte PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE     = 0;

  public static final short WUCLASS_NUMERIC_CONTROLLER                         = 3;
  public static final byte PROPERTY_NUMERIC_CONTROLLER_OUTPUT                  = 0;

  public static final short WUCLASS_LIGHT                                      = 4;
  public static final byte PROPERTY_LIGHT_ONOFF                                = 0;

  public static final short WUCLASS_LIGHT_SENSOR                               = 5;
  public static final byte PROPERTY_LIGHT_SENSOR_CURRENT_VALUE                 = 0;
  public static final byte PROPERTY_LIGHT_SENSOR_SAMPLING_RATE                 = 1;

  public static final short WUCLASS_AND_GATE                                   = 6;
  public static final byte PROPERTY_AND_GATE_IN1                               = 0;
  public static final byte PROPERTY_AND_GATE_IN2                               = 1;
  public static final byte PROPERTY_AND_GATE_OUTPUT                            = 2;
}

