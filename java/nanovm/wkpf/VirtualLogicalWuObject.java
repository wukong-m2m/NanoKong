package nanovm.wkpf;

public class VirtualLogicalWuObject extends GENERATEDVirtualLogicalWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        boolean in1 = WKPF.getPropertyBoolean(this, INPUT1);
        boolean in2 = WKPF.getPropertyBoolean(this, INPUT2);
        boolean in3 = WKPF.getPropertyBoolean(this, INPUT3);
        boolean in4 = WKPF.getPropertyBoolean(this, INPUT4);
        short operator = WKPF.getPropertyShort(this, OPERATOR);


      	if ( ((operator==WKPF.ENUM_LOGICAL_OPERATOR_AND) && (in1 & in2 & in3 & in4)) ||
             ((operator==WKPF.ENUM_LOGICAL_OPERATOR_OR) && (in1 | in2 | in3 | in4)) ||
             ((operator==WKPF.ENUM_LOGICAL_OPERATOR_NOT) && (!in1)) ||	//only not input1
             ((operator==WKPF.ENUM_LOGICAL_OPERATOR_XOR) && (in1 ^ in2 ^ in3 ^ in4)) 
		) {
            WKPF.setPropertyBoolean(this, OUTPUT, true);
            //System.out.println("WKPFUPDATE(Logical): in1 " + in1 + " in2 " + in2 + " in3 " + in3 + " in4 " + in4 + " operator " + operator + " -> TRUE");
        } else {
            WKPF.setPropertyBoolean(this, OUTPUT, false);
           // System.out.println("WKPFUPDATE(Logical): in1 " + in1 + " in2 " + in2 + " in3 " + in3 + " in4 " + in4 + " operator " + operator + " -> FALSE");
        }
    }
}
