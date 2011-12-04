/*
  LedBlink 2.java
 */

import nanovm.avr.*;

class LedBlink2 {
  public static void main(String[] args) {
    System.out.println("Blinking LED 2");
    System.out.println("System clock = " + AVR.getClock() + "000 Hz");

    // both led ports are outputs
    for(int pin = 2; pin < 14; pin++) 
      AVR.setPinIOMode(pin, 0);
    System.out.println("Set Output already");
    
    // no need to setup timer, since 100Hz is default ...

    while(true) {
      // PortB.0/1 for Demoboard
      for(int pin = 2; pin < 14; pin++)
        AVR.digitalWrite(pin, 1);
      System.out.println("On");
      Timer.wait(100);
      for(int pin = 2; pin < 14; pin++)
        AVR.digitalWrite(pin, 0);
      System.out.println("Off");
      Timer.wait(100);
    }
  }
}
