package nanovm.wkpf;

public class WKPF {
  public static native byte getErrorCode(); // Since we can't raise exceptions, I'll use this to signal whether a call succeeds or not.

  // Profile and endpoint maintenance
  public static native void registerProfile(short profileId, byte[] properties, byte numberOfProperties); // TODONR: numberOfProperties shouldn't be necessary, but I can't figure out how to get the array size in native code (need heap ID)
  public static native void createEndpoint(short profileId, byte portNumber, VirtualProfile virtualProfileInstance); // byte or short? No unsigned byte in Java ><
  public static native void removeEndpoint(byte portNumber);

  // Property access for virtual profiles
  public static native short getPropertyShort(VirtualProfile virtualProfileInstance, byte propertyNumber);
  public static native void setPropertyShort(VirtualProfile virtualProfileInstance, byte propertyNumber, short value);
  public static native boolean getPropertyBoolean(VirtualProfile virtualProfileInstance, byte propertyNumber);
  public static native void setPropertyBoolean(VirtualProfile virtualProfileInstance, byte propertyNumber, boolean value);

  // Property setters used in propertyDispatch
  public static native void setPropertyShort(short nodeId, byte portNumber, byte propertyNumber, short profileId, short value);
  public static native void setPropertyBoolean(short nodeId, byte portNumber, byte propertyNumber, short profileId, boolean value);

  // Select function for main loop
  public static native VirtualProfile select();

  // Getting the properties which values have to be propagated in propertyDispatch
  public static native boolean loadNextDirtyProperty();
  public static native byte getDirtyPropertyPortNumber();
  public static native byte getDirtyPropertyNumber();
  public static native short getDirtyPropertyShortValue();
  public static native boolean getDirtyPropertyBooleanValue();

  // Who am I?
  public static native short getMyNodeId();

  // Note: need to match definitions in wkpf.h
  public static final byte PROPERTY_TYPE_SHORT                         = 0;
  public static final byte PROPERTY_TYPE_BOOLEAN                       = 1;
  public static final byte PROPERTY_ACCESS_READ           = (byte)(1 << 7);
  public static final byte PROPERTY_ACCESS_WRITE          = (byte)(1 << 6);
  public static final byte PROPERTY_ACCESS_RW = (PROPERTY_ACCESS_READ|PROPERTY_ACCESS_WRITE);

  public static final byte OK                                     = 0;
  public static final byte ERR_ENDPOINT_NOT_FOUND                 = 1;
  public static final byte ERR_PROPERTY_NOT_FOUND                 = 2;
  public static final byte ERR_PROFILE_NOT_FOUND                  = 3;
  public static final byte ERR_READ_ONLY                          = 4;
  public static final byte ERR_WRITE_ONLY                         = 5;
  public static final byte ERR_PORT_IN_USE                        = 6;
  public static final byte ERR_PROFILE_ID_IN_USE                  = 7;
  public static final byte ERR_OUT_OF_MEMORY                      = 8;
  public static final byte ERR_WRONG_DATATYPE                     = 9;
  public static final byte ERR_ENDPOINT_ALREADY_ALLOCATED        = 10;
  public static final byte ERR_NEED_VIRTUAL_PROFILE_INSTANCE     = 11;
  public static final byte ERR_SHOULDNT_HAPPEN           = (byte)0xFF;

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

  public static final short PROFILE_TEMPERATURE_SENSOR                         = 2;
  public static final byte PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE     = (WKPF.PROP_BASE+0);

  public static final short PROFILE_NUMERIC_CONTROLLER                         = 3;
  public static final byte PROPERTY_NUMERIC_CONTROLLER_OUTPUT                  = (WKPF.PROP_BASE+0);

  public static final short PROFILE_LIGHT                                      = 4;
  public static final byte PROPERTY_LIGHT_ONOFF                                = (WKPF.PROP_BASE+0);

  public static final short PROFILE_LIGHT_SENSOR                               = 5;
  public static final byte PROPERTY_LIGHT_SENSOR_CURRENT_VALUE                 = (WKPF.PROP_BASE+0);
}
