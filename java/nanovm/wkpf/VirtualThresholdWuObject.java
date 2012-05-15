package nanovm.wkpf;

public class VirtualThresholdWuObject extends GENERATEDVirtualThresholdWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short operator = WKPF.getPropertyShort(this, OPERATOR);
        short threshold = WKPF.getPropertyShort(this, THRESHOLD);
        short value = WKPF.getPropertyShort(this, VALUE);

      	if (((operator == WKPF.ENUM_THRESHOLD_OPERATOR_GT || operator == WKPF.ENUM_THRESHOLD_OPERATOR_GTE) && value > threshold)
      	 || ((operator == WKPF.ENUM_THRESHOLD_OPERATOR_LT || operator == WKPF.ENUM_THRESHOLD_OPERATOR_LTE) && value < threshold)
      	 || ((operator == WKPF.ENUM_THRESHOLD_OPERATOR_GTE || operator == WKPF.ENUM_THRESHOLD_OPERATOR_LTE) && value == threshold)) {
            WKPF.setPropertyBoolean(this, OUTPUT, true);
            System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator + " -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, OUTPUT, false);
            System.out.println("WKPFUPDATE(Threshold): threshold " + threshold + " value " + value + " operator " + operator + " -> FALSE");
        }
    }
}
