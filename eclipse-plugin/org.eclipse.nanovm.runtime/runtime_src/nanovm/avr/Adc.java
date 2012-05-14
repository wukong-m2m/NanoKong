//
// nanovm/avr/Adc.java
//

package nanovm.avr;

public class Adc {
  
  // prescaler constants
  public static final int DIV2   = 0;
  public static final int DIV2A  = 1;  // no real use for this
  public static final int DIV4   = 2;
  public static final int DIV8   = 3;
  public static final int DIV16  = 4;
  public static final int DIV32  = 5;
  public static final int DIV64  = 6;
  public static final int DIV128 = 7;

  // set ADC prescaler
  static public native void setPrescaler(int value);

  // voltage reference constants
  public static final int AREF     = 0 << 6;
  public static final int AVCC     = 1 << 6;
//  public static final int RESERVED = 2 << 6;  // don't use
  public static final int INTERNAL = 3 << 6;  

  // set voltage reference
  static public native void setReference(int value);

  // channel constants
  public static final int CHANNEL0   = 0;
  public static final int CHANNEL1   = 1;
  public static final int CHANNEL2   = 2;
  public static final int CHANNEL3   = 3;
  public static final int CHANNEL4   = 4;
  public static final int CHANNEL5   = 5;
  public static final int CHANNEL6   = 6;
  public static final int CHANNEL7   = 7;
  public static final int CHANNEL8   = 32;
  public static final int CHANNEL9   = 33;
  public static final int CHANNEL10  = 34;
  public static final int CHANNEL11  = 35;
  public static final int CHANNEL12  = 36;
  public static final int CHANNEL13  = 37;
  public static final int CHANNEL14  = 38;
  public static final int CHANNEL15  = 39;
  public static final int Diff32x10 = 13;   //(channel3-channel2) *10 
  public static final int Diff32x200 = 15;  //(channel3-channel2) *200

  // get current ADC value
  static public native int getValue(int channel);  // get 10 bit value
  static public native int getByte(int channel);   // get 8 bit value
}
