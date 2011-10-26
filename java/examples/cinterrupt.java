/*
  inputDemo.java
 */

import nanovm.avr.*;

class cinterrupt {
  public static void main(String[] args) {

		//connect LED between GND and portA0(digital 22)
		AVR.portD.setInput(0);	//INT0
		AVR.portD.setBit(0);	//pull high
		AVR.portB.setInput(0);	//PCINT0
		AVR.portB.setBit(0);	//pull high
		AVR.portA.setOutput(0);
		AVR.portA.clrBit(0);	//initial A to low
		AVR.portD.setOutput(1);	
		while(true)
		{
			//connect portD1(digital 20) with PCINT0(digital 53) can see interrupt
			AVR.portD.clrBit(1);	
			Timer.wait(1000);
			System.out.println("touch portD0(digital 21) with GND to create interrupt (INT0)");");
			AVR.portD.setBit(1);
			Timer.wait(1000);
		}

  }
}


