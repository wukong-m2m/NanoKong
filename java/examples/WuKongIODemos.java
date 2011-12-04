import nanovm.avr.*;
class WuKongIODemos {
  public void SetDigitalOutput() {
    // Note that Port A to Port L, of which has 8 bits (P[A-L][0-7]), could be set
    // to operate as digital output, though Port F and Port K are analog ports
    // For example, Set PA0 to be output and output the low value
    AVR.portA.setOutput(0);
    AVR.portA.setBit(0);
    AVR.portA.clrBit(0);

    // Second Method: Set pin 13 to Output, and write 1 into the pin
    int pin = 13;
    AVR.setPinIOMode(pin, 0);
    AVR.digitalWrite(pin, 1);

    // ======= Comments =======
    // For now this works through calling AVR.portA. In the final version this should be
    // replaced with something that's not in a class AVR, since not all nodes will be AVR
    // based, and something that doesn't hardcode the IO ports, since different nodes will
    // have different numbers of IO ports.
    // This goes for all IO functions. Note that the first change is not much more than
    // changing some names. The second one requires us to think about the API more carefully.
  }

  public void ReadDigitalInput() {
    // Get input data from PA0
    AVR.portA.setInput(0);
    System.out.println("Value portA0: " + AVR.portA.getInput(0));   
    
    // Second Method: Set pin 13 to Iutput, and read from the pin
    int pin = 13;
    AVR.setPinIOMode(pin, 1);
    System.out.println("Value pin"+pin+": "+AVR.digitalRead(pin));
    
    // ======= Comments =======
    // The easiest way should be to just poll, but it would be useful to also be able
    // to provide a callback that will be triggered when a pin's value changes.
  }

  public void ReadAnalogInput() {
    // Using ADC to read analog input :
    // First, initialize the ADC conversion (there's no default setup for ADC)
    // DIV[2,4,...,128], DIV128 is most accurate but need most time to convert.
    Adc.setPrescaler(Adc.DIV64);
    
    // use internal reference voltage which is currently 2.56v
    // if necessary, we can extend it to use another internal reference 1.1v, 
    // external default reference (Vcc, 5v), or external reference connected to 
    // AREF pin which is at most Vcc.
    Adc.setReference(Adc.INTERNAL);
        
    // getValue returns value = Vin * 1024 / Vref   (Vin = voltage input, and 
    // Vref = voltage reference). If necessary, We can set a differential input 
    // pairs between two channels.
    // First, get voltage difference between channel 3 and channel 2.
    // if neceesary, there are some other difference of voltage to implement.
    System.out.println("Value (ch3-ch2)*10: " + Adc.getValue(Adc.Diff32x10));
    
    // Second, display ADC0 value with 10 bits resolution
    // ADC[0-7] is PortF.[0-7] (or Analog pin [0-7]) on mega2560 board
    System.out.println("Value CH0: " + Adc.getValue(Adc.CHANNEL0));
    // Third, display ADC0 value with 8 bits resolution
    System.out.println("Byte CH0:  " + Adc.getByte(Adc.CHANNEL0));
  }

  // SetAnalogOutput()
  public void SetPWM() {
    // Using PWM to write analog value as digital form.
    // Currently, we only have pwm0 (PB.4 or Digital pin 10),
    // and pwm1 (PH.6 or Digital pin 9).
    // DIVxx is a parameter to divide the system frequency.
    // ex. ( Arduino 2560 system freq is 16M Hz) 16M / 1024 = 16 Khz
    AVR.pwm0.setPrescaler(Pwm.DIV1024);

    int outvalue = 50;
    // use setRatio to control on and off states to output analog value
    AVR.pwm0.setRatio(outvalue);
  }
  
  public void SetTimer(){
    // ( Arduino 2560 system freq = 16M Hz) 
    // default base freq = 1/8 * system freq = 2Mhz
    // i.e. Timer.setPrescaler(Timer.DIV8);
    // Don't change default prescaler unless you know what you are doing
    
    // timer = 1/2000 * base freq -> 2M / 2000 = 1000 Hz -> T = 1ms
    Timer.setSpeed(2000); 
	System.out.println("wait 1 second");
	Timer.wait(1000);

	// timer = 1/4000 * base freq -> 2M / 4000 = 500 Hz -> T = 2ms	
    Timer.setSpeed(4000);    
    System.out.println("wait 1 second");
    Timer.wait(500);
    
    // timer = 1/4000 * base freq -> 2M / 4000 = 500 Hz -> T = 2ms
	Timer.setSpeed(4000);   
    System.out.println("wait 2 second");
    Timer.wait(1000);
  }
  
  // ======= Comments =======
  // ... whatever else we can do with the CPU. interupts, UART
}
