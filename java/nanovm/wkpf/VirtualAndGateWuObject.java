package nanovm.wkpf;

public class VirtualAndGateWuObject extends GENERATEDVirtualAndGateWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        boolean in1 = WKPF.getPropertyBoolean(this, IN1);
        boolean in2 = WKPF.getPropertyBoolean(this, IN2);

      	if (in1 && in2) {
            WKPF.setPropertyBoolean(this, OUTPUT, true);
            System.out.println("WKPFUPDATE(ANDGate):and gate -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, OUTPUT, false);
            System.out.println("WKPFUPDATE(ANDGate):and gate -> FALSE");
        }
    }
}
