/*
  LedBlink.java
 */

import nanovm.avr.*;

class ReferenceToNativeObjectTest {
  public static void main(String[] args) {
//	System.out.println("System clock = " + AVR.getClock() + "000 Hz");
//	System.out.println("Blinking LED");

//	Port port = AVR.portA;

    AVR.portA.setOutput(0);
    AVR.portA.setOutput(1);
    AVR.portA.setOutput(2);
    AVR.portA.setOutput(3);
    AVR.portA.setOutput(4);
    AVR.portA.setOutput(5);
    AVR.portA.setOutput(6);
    AVR.portA.setOutput(7);
    
	AVR.portA.setBit(0);
	AVR.portA.setBit(1);
	AVR.portA.setBit(2);
	AVR.portA.setBit(3);
	AVR.portA.setBit(4);
	AVR.portA.setBit(5);
	AVR.portA.setBit(6);
	AVR.portA.setBit(7);
  }
}


// Result: normal