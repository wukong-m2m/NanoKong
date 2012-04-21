package nanovm.wkpf;

public class VirtualANDGateProfile extends VirtualProfile {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_AND_GATE_IN1
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_AND_GATE_IN2
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ // PROPERTY_AND_GATE_OUTPUT
    };

    public static final short PROFILE_AND_GATE                                   = 0x1006;
    public static final byte PROPERTY_AND_GATE_IN1                               = (WKPF.PROP_BASE+0);
    public static final byte PROPERTY_AND_GATE_IN2                               = (WKPF.PROP_BASE+1);
    public static final byte PROPERTY_AND_GATE_OUTPUT                            = (WKPF.PROP_BASE+2);

    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualProfile once we get the inheritance issue sorted out.
        boolean in1 = WKPF.getPropertyBoolean(this, VirtualANDGateProfile.PROPERTY_AND_GATE_IN1);
        boolean in2 = WKPF.getPropertyBoolean(this, VirtualANDGateProfile.PROPERTY_AND_GATE_IN2);

      	if (in1 && in2) {
            WKPF.setPropertyBoolean(this, VirtualANDGateProfile.PROPERTY_AND_GATE_OUTPUT, true);
            System.out.println("and gate -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, VirtualANDGateProfile.PROPERTY_AND_GATE_OUTPUT, false);
            System.out.println("and gate -> FALSE");
        }
    }
}
