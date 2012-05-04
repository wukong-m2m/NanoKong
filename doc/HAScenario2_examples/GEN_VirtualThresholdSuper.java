package nanovm.wkpf;

public class GEN_VirtualThresholdSuper extends VirtualProfile {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_THRESHOLD_OPERATOR
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_THRESHOLD_THRESHOLD
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_THRESHOLD_VALUE
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ // PROPERTY_THRESHOLD_OUTPUT
    };

    public static final short OPERATOR_GT = 0;
    public static final short OPERATOR_LT = 1;
    public static final short OPERATOR_GTE = 2;
    public static final short OPERATOR_LTE = 3;
}
