/*
  examples.java
 */

import nanovm.avr.*;
import java.io.*;

class examples {
  public static void main(String[] args) throws IOException {

    /*** For Console output and input ***/
    System.out.println("NanoVM - console echo demo");
    System.out.println("System clock = " + AVR.getClock() + "000 Hz");
    System.out.println("Please press a key ...");

    while(true) {
        if(System.in.available() != 0){
            System.out.print((char) System.in.read());
            break;
        }
    }
    System.out.println("");
    
    /*** For Digital pins ***/
    // Output parts
    for (int i = 0; i < 7; ++i){
        AVR.portA.setOutput(i); // Set the port A.i to be output port
        AVR.portA.setBit(i);    // Pull the port to be High or On state
        Timer.wait(500);           
        AVR.portA.clrBit(i);    // Clear the port to be Low or Off state
        Timer.wait(500);
    }
    
    // Input parts
    for (int i = 0; i < 7; ++i){
        AVR.portA.setInput(i);
        //Leave the term to be determined, just demo the procedure
        //if(?.available() != 0)
        //      System.out.print((char) ?.read()); or ?.getValue()
    }
    
    /*** For Analog pins ***/
    // Input parts
    // initialize the ADC conversion (there's no default setup for ADC)
    Adc.setPrescaler(Adc.DIV64);     // slow but accurate conversion
    Adc.setReference(Adc.INTERNAL);  // use internal reference
    
    // display internal VBG voltage (about 1.23 volts)
    System.out.println("Value VBG: " + Adc.getValue(Adc.CHANNELVBG));

    // display internal GND (0 volts)
    System.out.println("Value Gnd: " + Adc.getValue(Adc.CHANNELGND));

    // display ADC0 value with 10 bits resolution
    System.out.println("Value CH0: " + Adc.getValue(Adc.CHANNEL0));

    // display ADC0 value with 8 bits resolution
    System.out.println("Byte CH0:  " + Adc.getByte(Adc.CHANNEL0));

    // wait a second for next run
    Timer.wait(10000);
    
    // Output parts
    // Using pwm? or Adc.setValue(Adc.CHANNEL0, value)?
   
  }
}
