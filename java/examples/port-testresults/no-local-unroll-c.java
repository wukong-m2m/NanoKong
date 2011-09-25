/*
  LedBlink.java
 */

import nanovm.avr.*;

class ReferenceToNativeObjectTest {
  public static void main(String[] args) {
//	System.out.println("System clock = " + AVR.getClock() + "000 Hz");
//	System.out.println("Blinking LED");

	Port port = AVR.portC;

    port.setOutput(0);
    port.setOutput(1);
    port.setOutput(2);
    port.setOutput(3);
    port.setOutput(4);
    port.setOutput(5);
    port.setOutput(6);
    port.setOutput(7);
    
	port.setBit(0);
	port.setBit(1);
	port.setBit(2);
	port.setBit(3);
	port.setBit(4);
	port.setBit(5);
	port.setBit(6);
	port.setBit(7);
  }
}


// Result: normal