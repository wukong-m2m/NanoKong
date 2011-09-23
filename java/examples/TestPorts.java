/*
  LedBlink.java
 */

import nanovm.avr.*;

class TestPorts {
  public static void main(String[] args) {
    System.out.println("Blinking LED");

    System.out.println("System clock = " + AVR.getClock() + "000 Hz");

    // both led ports are outputs
    AVR.portA.setOutput(0);
    AVR.portA.setOutput(1);
    AVR.portA.setOutput(2);
    AVR.portA.setOutput(3);
    AVR.portA.setOutput(4);
    AVR.portA.setOutput(5);
    AVR.portA.setOutput(6);
    AVR.portA.setOutput(7);
    
    // no need to setup timer, since 100Hz is default ...

    while(true) {
      // portA.0/1 for Demoboard
      AVR.portA.setBit(0);
      AVR.portA.setBit(1);
      AVR.portA.setBit(2);
      AVR.portA.setBit(3);
      AVR.portA.setBit(4);
      AVR.portA.setBit(5);
      AVR.portA.setBit(6);
      AVR.portA.setBit(7);
      Timer.wait(500);
      AVR.portA.clrBit(0);
      AVR.portA.clrBit(1);
      AVR.portA.clrBit(2);
      AVR.portA.clrBit(3);
      AVR.portA.clrBit(4);
      AVR.portA.clrBit(5);
      AVR.portA.clrBit(6);
      AVR.portA.clrBit(7);
      Timer.wait(500);
    }
  }
}
