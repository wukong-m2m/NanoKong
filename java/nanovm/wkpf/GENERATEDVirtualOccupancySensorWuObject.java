package nanovm.wkpf;

public abstract class GENERATEDVirtualOccupancySensorWuObject extends VirtualWuObject {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW // PROPERTY_OCCUPANCY_SENSOR_OCCUPIED
    };

    protected static final byte OCCUPIED                               = 0;
}
