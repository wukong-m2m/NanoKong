package nanovm.wkpf;

public class VirtualThresholdWuObject extends VirtualWuObject {
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

    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short operator = WKPF.getPropertyShort(this, WKPF.PROPERTY_THRESHOLD_OPERATOR);
        short threshold = WKPF.getPropertyShort(this, WKPF.PROPERTY_THRESHOLD_THRESHOLD);
        short value = WKPF.getPropertyShort(this, WKPF.PROPERTY_THRESHOLD_VALUE);

      	if (((operator == OPERATOR_GT || operator == OPERATOR_GTE) && value > threshold)
      	 || ((operator == OPERATOR_LT || operator == OPERATOR_LTE) && value < threshold)
      	 || ((operator == OPERATOR_GTE || operator == OPERATOR_LTE) && value == threshold)) {
            WKPF.setPropertyBoolean(this, WKPF.PROPERTY_THRESHOLD_OUTPUT, true);
            System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator + " -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, WKPF.PROPERTY_THRESHOLD_OUTPUT, false);
            System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator + " -> FALSE");
        }
    }
}
