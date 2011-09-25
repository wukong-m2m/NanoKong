/*
  LedBlink.java
 */

import nanovm.avr.*;

class ReferenceToNativeObjectTest {
  public static void main(String[] args) {
//	System.out.println("System clock = " + AVR.getClock() + "000 Hz");
//	System.out.println("Blinking LED");

//	Port port = AVR.portA;

    AVR.portC.setOutput(0);
    AVR.portC.setOutput(1);
    AVR.portC.setOutput(2);
    AVR.portC.setOutput(3);
    AVR.portC.setOutput(4);
    AVR.portC.setOutput(5);
    AVR.portC.setOutput(6);
    AVR.portC.setOutput(7);
    
	AVR.portC.setBit(0);
	AVR.portC.setBit(1);
	AVR.portC.setBit(2);
	AVR.portC.setBit(3);
	AVR.portC.setBit(4);
	AVR.portC.setBit(5);
	AVR.portC.setBit(6);
	AVR.portC.setBit(7);
  }
}


// Result: normal