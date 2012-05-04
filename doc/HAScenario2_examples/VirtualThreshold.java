package nanovm.wkpf;

public class VirtualThreshold extends GEN_VirtualThresholdSuper {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualProfile once we get the inheritance issue sorted out.
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
