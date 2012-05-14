package nanovm.wkpf;

public class GENERATEDVirtualThresholdWuObject extends VirtualWuObject {
    public static final byte[] properties = new byte[] {
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_THRESHOLD_OPERATOR
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_THRESHOLD_THRESHOLD
            WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW, // PROPERTY_THRESHOLD_VALUE
            WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ // PROPERTY_THRESHOLD_OUTPUT
    };

    protected static final byte OPERATOR                                = 0;
    protected static final byte THRESHOLD                               = 1;
    protected static final byte VALUE                                   = 2;
    protected static final byte OUTPUT                                  = 3;
}
