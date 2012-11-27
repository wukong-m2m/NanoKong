package nanovm.wkpf;

public class VirtualMathOpWuObject extends GENERATEDVirtualMathOpWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short input1 = WKPF.getPropertyShort(this, INPUT1);
        short input2 = WKPF.getPropertyShort(this, INPUT2);
        short input3 = WKPF.getPropertyShort(this, INPUT3);
        short input4 = WKPF.getPropertyShort(this, INPUT4);
        short operator = WKPF.getPropertyShort(this, OPERATOR);
	short output=0;
	short remainder=0;


      	if ( operator==WKPF.ENUM_MATH_OPERATOR_MAX ) {
		if ( (input1>=input2) && (input1>=input3) && (input1>=input4))
			output=input1;
		else if ( (input2>=input1) && (input2>=input3) && (input2>=input4))
			output=input2;
		else if ( (input3>=input1) && (input3>=input2) && (input3>=input4))
			output=input3;
		else if ( (input4>=input1) && (input4>=input2) && (input4>=input3))
			output=input4;
		remainder=0;
        } else if ( operator==WKPF.ENUM_MATH_OPERATOR_MIN ) {
	  	if ( (input1<=input2) && (input1<=input3) && (input1<=input4))
			output=input1;
		else if ( (input2<=input1) && (input2<=input3) && (input2<=input4))
			output=input2;
		else if ( (input3<=input1) && (input3<=input2) && (input3<=input4))
			output=input3;
		else if ( (input4<=input1) && (input4<=input2) && (input4<=input3))
			output=input4;
		remainder=0;
        } else if(operator==WKPF.ENUM_MATH_OPERATOR_AVG) {
		output=(short)((input1+input2+input3+input4)/4);
		remainder=0;
        } else if(operator==WKPF.ENUM_MATH_OPERATOR_ADD) {
		output=(short)(input1+input2+input3+input4);
		remainder=0;
        } else if(operator==WKPF.ENUM_MATH_OPERATOR_SUB) {	// input1-input2
		output=(short)(input1-input2);
		remainder=0;
        } else if(operator==WKPF.ENUM_MATH_OPERATOR_MULTIPLY) {
		output=(short)(input1*input2*input3*input4);
		remainder=0;
        } else if(operator==WKPF.ENUM_MATH_OPERATOR_DIVIDE) {	// input1/input2
		if(input2==0)
			System.out.println("WKPFUPDATE(math):divide by 0 Error");
		output=(short)(input1/input2);
		remainder=(short)(input1%input2);
        }

        WKPF.setPropertyShort(this, OUTPUT, output);
        WKPF.setPropertyShort(this, REMAINDER, remainder);
        System.out.println("WKPFUPDATE(math): in1 " + input1 + " in2 " + input2 + " in3 " + input3 + " in4 " + input4 + " operator " + operator + " output " + output);
    }
}
