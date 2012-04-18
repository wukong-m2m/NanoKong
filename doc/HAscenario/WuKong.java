public abstract class VirtualProfile {
	protected int getPropertyInt(int propertyId) { ... }
	protected boolean getPropertyBoolean(int propertyId) { ... }
	protected void setPropertyInt(int propertyId, int value) { ... }
	protected void setPropertyBoolean(int propertyId, boolean value) { ... }
	
	public abstract void update();
	public abstract byte[] getDefinition() 
}

public final class Endpoint { // An endpoint is identified by it's node address and a port number
	public int nodeAddress;
	public int portNo;
}

public final class DirtyProperty {
	public EndPoint endpoint;
	public int propertyId;
	public int valueInt;
	public boolean valueBoolean;
}

public final static class WKPF {
	// Constants for virtual profile definitions
	// 1 byte per property: bits 7 and 6 indicate r/w access, bit 0 indicates datatype (only have 2 at the moment), bits 1 through 5 aren't used.
	public static final int PROPERTY_TYPE_INT = 0;
	public static final int PROPERTY_TYPE_BOOLEAN = 1;
	public static final int PROPERTY_ACCESS_RO = 2 << 6;
	public static final int PROPERTY_ACCESS_WO = 1 << 6;
	public static final int PROPERTY_ACCESS_RW = 3 << 6;
	
	// Profile IDs
	public static final int PROFILEDEF_TEMPERATURESENSOR = 0;
	public static final int PROFILEDEF_NUMERICCONTROLLER = 1;
	public static final int PROFILEDEF_LIGHT = 2;

	// Property IDs	
	public static final int PROFILEDEF_GENERAL_REFRESHRATE = 0; // Common for all profiles

	public static final int PROFILEDEF_TEMPERATURESENSOR_CURRENTTEMPERATURE = BASE+1;

	public static final int PROFILEDEF_NUMERICCONTROLLER_OUT = 1;

	public static final int PROFILEDEF_LIGHT_ONOFF = 1;


	public static int getMyNodeId() { ... }
	public static void addProfile(VirtualProfile profile, int endpointPortNo) { ... }
	public static void setPropertyInt(EndpointId endpoint, int propertyId, int value) { ... }
	public static void setPropertyBoolean(EndpointId endpoint, int propertyId, boolean value) { ... }
	public static DirtyProperty getNextDirtyProperty() { ... }
	public static WuKongVirtualProfile wait() { ... }
}
