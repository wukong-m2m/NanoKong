class IOExamples {
  public void SetDigitalOutput() {
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
    
    // The easiest way should be to just poll, but it would be useful to also be able
    // to provide a callback that will be triggered when a pin's value changes.
  }

  public void ReadAnalogInput() {
    // ....?
  }

  public void SetPWM() {
    // ....?
  }
  
  // ... whatever else we can do with the CPU. Timers, interupts, UART
}