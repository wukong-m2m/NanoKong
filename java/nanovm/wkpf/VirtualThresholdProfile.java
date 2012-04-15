package nanovm.wkpf;

public class VirtualThresholdProfile extends VirtualProfile {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_ID_THRESHOLD_OPERATOR
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_ID_THRESHOLD_THRESHOLD
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_ID_THRESHOLD_VALUE
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ // PROPERTY_ID_THRESHOLD_OUTPUT
    };

    public static final short THRESHOLD_PROFILE_OPERATOR_GT = 0;
    public static final short THRESHOLD_PROFILE_OPERATOR_LT = 1;

    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualProfile once we get the inheritance issue sorted out.
        short operator = WKPF.getPropertyShort(this, WKPF.PROPERTY_ID_THRESHOLD_OPERATOR);
        short threshold = WKPF.getPropertyShort(this, WKPF.PROPERTY_ID_THRESHOLD_THRESHOLD);
        short value = WKPF.getPropertyShort(this, WKPF.PROPERTY_ID_THRESHOLD_VALUE);

      	if ((operator == THRESHOLD_PROFILE_OPERATOR_GT && value > threshold)
      	 || (operator == THRESHOLD_PROFILE_OPERATOR_LT && value < threshold)) {
            WKPF.setPropertyBoolean(this, WKPF.PROPERTY_ID_THRESHOLD_OUTPUT, true);
        } else {
            WKPF.setPropertyBoolean(this, WKPF.PROPERTY_ID_THRESHOLD_OUTPUT, false);
        }
    }
}
