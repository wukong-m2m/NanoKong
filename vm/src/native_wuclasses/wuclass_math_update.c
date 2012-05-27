#include <debug.h>
#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_MATH

void wuclass_math_update(wkpf_local_wuobject *wuobject) {
  int16_t input1;
  int16_t input2;
  int16_t input3;
  int16_t input4;
  int16_t op;
  int16_t output = 0;
  int16_t remainder = 0;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_INPUT1, &input1);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_INPUT2, &input2);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_INPUT3, &input3);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_INPUT4, &input4);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_MATH_OPERATOR, &op);

  if(op==WKPF_ENUM_MATH_OPERATOR_MAX) {
		if ( (input1>=input2) && (input1>=input3) && (input1>=input4))
			output=input1;
		else if ( (input2>=input1) && (input2>=input3) && (input2>=input4))
			output=input2;
		else if ( (input3>=input1) && (input3>=input2) && (input3>=input4))
			output=input3;
		else if ( (input4>=input1) && (input4>=input2) && (input4>=input3))
			output=input4;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_MIN) {
	  	if ( (input1<=input2) && (input1<=input3) && (input1<=input4))
			output=input1;
		else if ( (input2<=input1) && (input2<=input3) && (input2<=input4))
			output=input2;
		else if ( (input3<=input1) && (input3<=input2) && (input3<=input4))
			output=input3;
		else if ( (input4<=input1) && (input4<=input2) && (input4<=input3))
			output=input4;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_AVG) {
		output=(input1+input2+input3+input4)/4;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_ADD) {
		output=input1+input2+input3+input4;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_SUB) {	// input1-input2
		output=input1-input2;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_MULTIPLY) {
		output=input1*input2*input3*input4;
		remainder=0;
  } else if(op==WKPF_ENUM_MATH_OPERATOR_DIVIDE) {	// input1/input2
		if(input2==0)
			DEBUGF_WKPFUPDATE("WKPFUPDATE(math): divide by 0 Error ");
	    output=input1/input2;
		remainder=input1%input2;
  }

    wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MATH_OUTPUT, output);
	wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_MATH_REMAINDER, remainder);
    DEBUGF_WKPFUPDATE("WKPFUPDATE(math): Native math: input1 %x input2 %x input3 %x input4 %x operator %x-> output %x remainder %x\n", input1, input2, input3, input4, op, output, remainder);
}

#endif
