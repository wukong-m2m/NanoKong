/*
  LedBlink.java
 */

import nanovm.avr.*;

class ReferenceToNativeObjectTest {
  public static void main(String[] args) {
		System.out.println("System clock = " + AVR.getClock() + "000 Hz");
		System.out.println("Blinking LED");

		Port port = AVR.portA;

		for(int i=0; i<8; i++)
	      port.setOutput(i);

	    for(int i=0; i<8; i++)
		  port.clrBit(i);
/*
	    port.setOutput(0);
	    port.setOutput(1);
	    port.setOutput(2);
	    port.setOutput(3);
	    port.setOutput(4);
	    port.setOutput(5);
	    port.setOutput(6);
	    port.setOutput(7);

		port.clrBit(0);
		port.clrBit(1);
		port.clrBit(2);
		port.clrBit(3);
		port.clrBit(4);
		port.clrBit(5);
		port.clrBit(6);
		port.clrBit(7);
*/
  }
}


// Result: not port A, but port C (!!) is on