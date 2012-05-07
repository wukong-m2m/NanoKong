package nanovm.wkpf;

public class VirtualANDGateProfile extends VirtualProfile {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_AND_GATE_IN1
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_AND_GATE_IN2
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ // PROPERTY_AND_GATE_OUTPUT
    };

    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualProfile once we get the inheritance issue sorted out.
        boolean in1 = WKPF.getPropertyBoolean(this, WKPF.PROPERTY_AND_GATE_IN1);
        boolean in2 = WKPF.getPropertyBoolean(this, WKPF.PROPERTY_AND_GATE_IN2);

      	if (in1 && in2) {
            WKPF.setPropertyBoolean(this, WKPF.PROPERTY_AND_GATE_OUTPUT, true);
            System.out.println("WKPFUPDATE(ANDGate):and gate -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, WKPF.PROPERTY_AND_GATE_OUTPUT, false);
            System.out.println("WKPFUPDATE(ANDGate):and gate -> FALSE");
        }
    }
}
