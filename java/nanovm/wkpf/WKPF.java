package nanovm.wkpf;

public class WKPF {
  public static native byte getErrorCode(); // Since we can't raise exceptions, I'll use this to signal whether a call succeeds or not.

  // Profile and endpoint maintenance
  public static native void registerProfile(short profileId, byte[] properties);
  public static native void createEndpoint(short profileId, byte portNumber, VirtualProfile virtualProfileInstance); // byte or short? No unsigned byte in Java ><
  public static native void removeEndpoint(byte portNumber);

  // Property access for virtual profiles
  public static native short getPropertyShort(VirtualProfile virtualProfileInstance, byte propertyNumber);
  public static native void setPropertyShort(VirtualProfile virtualProfileInstance, byte propertyNumber, short value);
  public static native boolean getPropertyBoolean(VirtualProfile virtualProfileInstance, byte propertyNumber);
  public static native void setPropertyBoolean(VirtualProfile virtualProfileInstance, byte propertyNumber, boolean value);

  // Property setters used to initialise properties
  public static native void setPropertyShort(short componentId, byte propertyNumber, short value);
  public static native void setPropertyBoolean(short componentId, byte propertyNumber, boolean value);

  // Select function for main loop
  public static native VirtualProfile select();

  // Load component-endpoint map and link definitions
  public static native void loadComponentToEndpointMap(byte[] map);
  public static native void loadLinkDefinitions(byte[] links);

  // Who am I?
  public static native short getMyNodeId();

  // Note: need to match definitions in wkpf.h
  public static final byte PROPERTY_TYPE_SHORT                         = 0;
  public static final byte PROPERTY_TYPE_BOOLEAN                       = 1;
  public static final byte PROPERTY_ACCESS_READ           = (byte)(1 << 7);
  public static final byte PROPERTY_ACCESS_WRITE          = (byte)(1 << 6);
  public static final byte PROPERTY_ACCESS_RW = (PROPERTY_ACCESS_READ|PROPERTY_ACCESS_WRITE);

  public static final byte OK                                             =  0;
  public static final byte ERR_ENDPOINT_NOT_FOUND                         =  1;
  public static final byte ERR_PROPERTY_NOT_FOUND                         =  2;
  public static final byte ERR_PROFILE_NOT_FOUND                          =  3;
  public static final byte ERR_READ_ONLY                                  =  4;
  public static final byte ERR_WRITE_ONLY                                 =  5;
  public static final byte ERR_PORT_IN_USE                                =  6;
  public static final byte ERR_PROFILE_ID_IN_USE                          =  7;
  public static final byte ERR_OUT_OF_MEMORY                              =  8;
  public static final byte ERR_WRONG_DATATYPE                             =  9;
  public static final byte ERR_ENDPOINT_ALREADY_ALLOCATED                 = 10;
  public static final byte ERR_NEED_VIRTUAL_PROFILE_INSTANCE              = 11;
  public static final byte ERR_NVMCOMM_SEND_ERROR                         = 12;
  public static final byte ERR_NVMCOMM_NO_REPLY                           = 13;
  public static final byte ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED = 14;
  public static final byte ERR_SHOULDNT_HAPPEN                    = (byte)0xFF;

  // Profile library
  public static final byte PROPERTY_COMMON_REFRESHRATE                         = 0; // TODO: implement this
  public static final byte PROP_BASE                                           = 0;

  public static final short PROFILE_GENERIC                                    = 0;
  public static final byte PROPERTY_GENERIC_DUMMY                              = (WKPF.PROP_BASE+0);

  public static final short PROFILE_THRESHOLD                                  = 1;
  public static final byte PROPERTY_THRESHOLD_OPERATOR                         = (WKPF.PROP_BASE+0);
  public static final byte PROPERTY_THRESHOLD_THRESHOLD                        = (WKPF.PROP_BASE+1);
  public static final byte PROPERTY_THRESHOLD_VALUE                            = (WKPF.PROP_BASE+2);
  public static final byte PROPERTY_THRESHOLD_OUTPUT                           = (WKPF.PROP_BASE+3);
  public static final short PROPERTY_THRESHOLD_OPERATOR_GT                     = 0;
  public static final short PROPERTY_THRESHOLD_OPERATOR_LT                     = 1;
  public static final short PROPERTY_THRESHOLD_OPERATOR_GTE                    = 2;
  public static final short PROPERTY_THRESHOLD_OPERATOR_LTE                    = 3;

  public static final short PROFILE_TEMPERATURE_SENSOR                         = 2;
  public static final byte PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE     = (WKPF.PROP_BASE+0);

  public static final short PROFILE_NUMERIC_CONTROLLER                         = 3;
  public static final byte PROPERTY_NUMERIC_CONTROLLER_OUTPUT                  = (WKPF.PROP_BASE+0);

  public static final short PROFILE_LIGHT                                      = 4;
  public static final byte PROPERTY_LIGHT_ONOFF                                = (WKPF.PROP_BASE+0);

  public static final short PROFILE_LIGHT_SENSOR                               = 5;
  public static final byte PROPERTY_LIGHT_SENSOR_CURRENT_VALUE                 = (WKPF.PROP_BASE+0);
}
