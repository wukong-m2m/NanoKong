/*
    ADCDemo.java

    Simple analog digital conversion (ADC) demo. Run this e.g. on the
    simple avr atmega8 demo board
 */

import nanovm.avr.*;

class AdcDemo {
  public static void main(String[] args) {
    System.out.println("Analog digital conversion demo");

    // no need to initialize the timer, since 100Hz is default.

    // initialize the ADC conversion (there's no default setup for ADC)
    Adc.setPrescaler(Adc.DIV64);     // slow but accurate conversion
    Adc.setReference(Adc.INTERNAL);  // use internal reference

    // permanently poll ADC channel0
    while(true) {
      // display voltage difference between channel3.channel2
      System.out.println("Value (ch3-ch2)*10: " + Adc.getValue(Adc.Diff32x10));

      // display ADC15 value with 10 bits resolution
      System.out.println("Value CH15: " + Adc.getValue(Adc.CHANNEL15));

      // display ADC15 value with 8 bits resolution
      System.out.println("Byte CH15:  " + Adc.getByte(Adc.CHANNEL15));

      // wait a second for next run
      Timer.wait(1000);
    }
  }
}

     
