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
	  port.setBit(i);
  }
}


// Result: first and second test: not port A, but port C (!!) is faint, 33-34s later turns to bright