package nanovm.wkpf;

public class WKPF {
    public static native byte getErrorCode(); // Since we can't raise exceptions, I'll use this to signal whether a call succeeds or not.

    // Profile and endpoint maintenance
    public static native void registerProfile(short profileId, byte[] properties, byte numberOfProperties); // TODONR: numberOfProperties shouldn't be necessary, but I can't figure out how to get the array size in native code (need heap ID)
    public static native void createEndpoint(short profileId, byte portNumber, VirtualProfile virtualProfileInstance); // byte or short? No unsigned byte in Java ><
    public static native void removeEndpoint(byte portNumber);

    // Property access
    public static native short getPropertyShort(VirtualProfile virtualProfileInstance, byte propertyNumber);

/*
        #define wkpf_internal_read_property_int16(endpoint, property_number, value)      wkpf_read_property_int16(endpoint, property_number, FALSE, value)
        #define wkpf_internal_write_property_int16(endpoint, property_number, value)     wkpf_write_property_int16(endpoint, property_number, FALSE, value)
        #define wkpf_internal_read_property_boolean(endpoint, property_number, value)    wkpf_read_property_boolean(endpoint, property_number, FALSE, value)
        #define wkpf_internal_write_property_boolean(endpoint, property_number, value)   wkpf_write_property_boolean(endpoint, property_number, FALSE, value)
*/



    
    // Select function for main loop
    public static native VirtualProfile select();

    // Note: need to match definitions in wkpf.h
	public static final byte PROPERTY_TYPE_SHORT                         = 0;
	public static final byte PROPERTY_TYPE_BOOLEAN                       = 1;
	public static final byte PROPERTY_ACCESS_READ           = (byte)(1 << 7);
	public static final byte PROPERTY_ACCESS_WRITE          = (byte)(1 << 6);
	public static final byte PROPERTY_ACCESS_RW = (PROPERTY_ACCESS_READ|PROPERTY_ACCESS_WRITE);

	public static final byte WKPF_OK                                     = 0;
/*	public static final byte WKPF_ERR_ENDPOINT_NOT_FOUND                 = 1;
	public static final byte WKPF_ERR_PROPERTY_NOT_FOUND                 = 2;
	public static final byte WKPF_ERR_PROFILE_NOT_FOUND                  = 3;
	public static final byte WKPF_ERR_READ_ONLY                          = 4;
	public static final byte WKPF_ERR_WRITE_ONLY                         = 5;
	public static final byte WKPF_ERR_PORT_IN_USE                        = 6;
	public static final byte WKPF_ERR_PROFILE_ID_IN_USE                  = 7;*/
	public static final byte WKPF_ERR_OUT_OF_MEMORY                      = 8;
/*	public static final byte WKPF_ERR_WRONG_DATATYPE                     = 9;
	public static final byte WKPF_ERR_ENDPOINT_ALREADY_ALLOCATED        = 10;
	public static final byte WKPF_ERR_SHOULDNT_HAPPEN                 = 0xFF;*/
}
