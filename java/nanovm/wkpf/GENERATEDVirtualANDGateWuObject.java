package nanovm.wkpf;

public abstract class GENERATEDVirtualANDGateWuObject extends VirtualWuObject {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_AND_GATE_IN1
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_AND_GATE_IN2
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ // PROPERTY_AND_GATE_OUTPUT
    };

    protected static final byte IN1                               = 0;
    protected static final byte IN2                               = 1;
    protected static final byte OUTPUT                            = 2;
}
