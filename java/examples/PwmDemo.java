/*
    PwmDemo.java
 */

import nanovm.avr.*;

class PwmDemo {
  public static void main(String[] args) {
    System.out.println("Pulse width modulation demo");

    AVR.pwm0.setPrescaler(Pwm.DIV1024);  // 8 Khz
 
    //pwm0 is set to OC2A, pwm1 is set to OC2B,
    while(true) {
      // led is connected to Vcc -> pwm set to 0 will light it 100%
      System.out.println("Going bright to dark");
      for(int i=0;i<256;i++) {
	AVR.pwm0.setRatio(i);
	Timer.wait(20); // wait 2ms
      }

      Timer.wait(5000); // wait 500ms

      System.out.println("Going dark to bright");
      for(int i=255;i>=0;i--) {
	AVR.pwm0.setRatio(i);
	Timer.wait(20); // wait 2ms
      }
      Timer.wait(5000); // wait 500ms
    }
  }
}

     
