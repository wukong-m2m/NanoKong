/*
  LedBlink.java
 */

import nanovm.avr.*;

class ReferenceToNativeObjectTest {
  public static void main(String[] args) {
//	System.out.println("System clock = " + AVR.getClock() + "000 Hz");
//	System.out.println("Blinking LED");

//	Port port = AVR.portA;

	for(int i=0; i<8; i++)
      AVR.portC.setOutput(i);
    
    for(int i=0; i<8; i++)
	  AVR.portC.setBit(i);
  }
}


// Result: normal