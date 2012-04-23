package nanovm.wkpf;

public class VirtualOccupancySensorProfile extends VirtualProfile {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW // PROPERTY_OCCUPANCY_SENSOR_OCCUPIED
    };

    public static final short PROFILE_OCCUPANCY_SENSOR                           = 0x1005;
    public static final byte PROPERTY_OCCUPANCY_SENSOR_OCCUPIED                  = (WKPF.PROP_BASE+0);

    public void update() {
      System.out.println("WKPFUPDATE(OccupancySensor): NOP");
      return;
    }
}
