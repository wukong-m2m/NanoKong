import nanovm.avr.*;
class WuKongIODemos {
  public void SetDigitalOutput() {
    // Note that Port A to Port L, of which has 8 bits, could be set in this way
    // to operate as digital output, though Port F and Port K are analog ports
    
    // Turn on bit 0 of digital port A
    AVR.portA.setOutput(0);
    AVR.portA.setBit(0);

    // For now this works through calling AVR.portA. In the final version this should be
    // replaced with something that's not in a class AVR, since not all nodes will be AVR
    // based, and something that doesn't hardcode the IO ports, since different nodes will
    // have different numbers of IO ports.
    // This goes for all IO functions. Note that the first change is not much more than
    // changing some names. The second one requires us to think about the API more carefully.
  }

  public void ReadDigitalInput() {
    AVR.portA.setInput(0);
    //get input data from PA0
    System.out.println("Value portA0: " + AVR.portA.getInput(0));     
    // The easiest way should be to just poll, but it would be useful to also be able
    // to provide a callback that will be triggered when a pin's value changes.
  }

  public void ReadAnalogInput() {
    // Using ADC to read analog input
    // initialize the ADC conversion (there's no default setup for ADC)
    Adc.setPrescaler(Adc.DIV64);     // slow but accurate conversion
    Adc.setReference(Adc.INTERNAL);  // use internal reference
    
    // display ADC0 value with 10 bits resolution
    // ADC[0-7] is PortF.[0-7] or Analog pin [0-7] on mega2560 board
    System.out.println("Value CH0: " + Adc.getValue(Adc.CHANNEL0));
    // display ADC0 value with 8 bits resolution
    System.out.println("Byte CH0:  " + Adc.getByte(Adc.CHANNEL0));
  }

  public void SetPWM() {
    // Using PWM to write analog value as digital form
    // Currently, we only have pwm0 (PB.4 or Digital pin 10), and pwm1 (PH.6 or Digital pin 9).
    AVR.pwm0.setPrescaler(Pwm.DIV1024);  // 8 Khz
    int outvalue = 50;
    AVR.pwm0.setRatio(outvalue); // use ratio btw on and off to represent output analog value
  }
  
  // ... whatever else we can do with the CPU. Timers, interupts, UART
}
