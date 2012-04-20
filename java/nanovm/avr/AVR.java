//
// nanovm/avr/AVR.java
//
// part of NanoVM
//

package nanovm.avr;

public class AVR {
  // these are fields representing ports the vm 
  // handles internally
  public static Port portA;
  public static Port portB;
  public static Port portC;
  public static Port portD;
  public static Port portE;
  public static Port portF;
  public static Port portG;
  public static Port portH;
  public static Port portJ;
  public static Port portK;
  public static Port portL;

  // and the PWM units
  public static Pwm pwm0;
  public static Pwm pwm1;
 
  // the USART units 
  public static USART usart0;
  public static USART usart1;
  public static USART usart2;
  public static USART usart3;

  // return system clock in khz
  static public native int getClock();
  static public native int setPinIOMode(int pin, int mode);
  static public native void digitalWrite(int pin, int value);
  static public native int digitalRead(int pin);
  static public native int setPinInterruptMode(int pin, int mode);
  static public native int select(int event_mask, int time);
  static public native void sleep(int time);

  //IO mode
  public static final int OUTPUT     = 0;
  public static final int INPUT	     = 1;
  public static final int LOW	     = 0;
  public static final int HIGH	     = 1;

  //interrupt mode
  public static final int GND        = 0;
  public static final int ANYCHANGE  = 1;
  public static final int NEGEDGE    = 2;
  public static final int POSEDGE    = 3;
  public static final int DISABLE    = 4;

  //event mask
  public static final int EVENT_INT   = 0;
  public static final int EVENT_PCINTA= 1;
  public static final int EVENT_PCINTB= 2;
  public static final int EVENT_PCINTC= 3;
  public static final int MASK_INT0   = 1;
  public static final int MASK_INT1   = 2;
  public static final int MASK_INT2   = 4;
  public static final int MASK_INT3   = 8;
  public static final int MASK_INT4   = 16;
  public static final int MASK_INT5   = 32;
  public static final int MASK_PCINT0 = 1;
  public static final int MASK_PCINT1 = 2;
  public static final int MASK_PCINT2 = 4;
  public static final int MASK_PCINT3 = 8;
  public static final int MASK_PCINT4 = 16;
  public static final int MASK_PCINT5 = 32;
  public static final int MASK_PCINT6 = 64;
  public static final int MASK_PCINT7 = 128;
  public static final int MASK_PCINT8 = 1;
  public static final int MASK_PCINT9 = 2;
  public static final int MASK_PCINT10= 4;
  public static final int MASK_PCINT11= 8;
  public static final int MASK_PCINT12= 16;
  public static final int MASK_PCINT13= 32;
  public static final int MASK_PCINT14= 64;
  public static final int MASK_PCINT15= 128;
  public static final int MASK_PCINT16= 1;
  public static final int MASK_PCINT17= 2;
  public static final int MASK_PCINT18= 4;
  public static final int MASK_PCINT19= 8;
  public static final int MASK_PCINT20= 16;
  public static final int MASK_PCINT21= 32;
  public static final int MASK_PCINT22= 64;
  public static final int MASK_PCINT23= 128;

  //pin constant
  public static final int DIGITAL0    = 0;
  public static final int DIGITAL1    = 1;
  public static final int DIGITAL2    = 2;
  public static final int DIGITAL3    = 3;
  public static final int DIGITAL4    = 4;
  public static final int DIGITAL5    = 5;
  public static final int DIGITAL6    = 6;
  public static final int DIGITAL7    = 7;
  public static final int DIGITAL8    = 8;
  public static final int DIGITAL9    = 9;
  public static final int DIGITAL10   = 10;
  public static final int DIGITAL11   = 11;
  public static final int DIGITAL12   = 12;
  public static final int DIGITAL13   = 13;
  public static final int DIGITAL14   = 14;
  public static final int DIGITAL15   = 15;
  public static final int DIGITAL16   = 16;
  public static final int DIGITAL17   = 17;
  public static final int DIGITAL18   = 18;
  public static final int DIGITAL19   = 19;
  public static final int DIGITAL20   = 20;
  public static final int DIGITAL21   = 21;
  public static final int DIGITAL22   = 22;
  public static final int DIGITAL23   = 23;
  public static final int DIGITAL24   = 24;
  public static final int DIGITAL25   = 25;
  public static final int DIGITAL26   = 26;
  public static final int DIGITAL27   = 27;
  public static final int DIGITAL28   = 28;
  public static final int DIGITAL29   = 29;
  public static final int DIGITAL30   = 30;
  public static final int DIGITAL31   = 31;
  public static final int DIGITAL32   = 32;
  public static final int DIGITAL33   = 33;
  public static final int DIGITAL34   = 34;
  public static final int DIGITAL35   = 35;
  public static final int DIGITAL36   = 36;
  public static final int DIGITAL37   = 37;
  public static final int DIGITAL38   = 38;
  public static final int DIGITAL39   = 39;
  public static final int DIGITAL40   = 40;
  public static final int DIGITAL41   = 41;
  public static final int DIGITAL42   = 42;
  public static final int DIGITAL43   = 43;
  public static final int DIGITAL44   = 44;
  public static final int DIGITAL45   = 45;
  public static final int DIGITAL46   = 46;
  public static final int DIGITAL47   = 47;
  public static final int DIGITAL48   = 48;
  public static final int DIGITAL49   = 49;
  public static final int DIGITAL50   = 50;
  public static final int DIGITAL51   = 51;
  public static final int DIGITAL52   = 52;
  public static final int DIGITAL53   = 53;
  public static final int ANALOG0     = 54;
  public static final int ANALOG1     = 55;
  public static final int ANALOG2     = 56;
  public static final int ANALOG3     = 57;
  public static final int ANALOG4     = 58;
  public static final int ANALOG5     = 59;
  public static final int ANALOG6     = 60;
  public static final int ANALOG7     = 61;
  public static final int ANALOG8     = 62;
  public static final int ANALOG9     = 63;
  public static final int ANALOG10    = 64;
  public static final int ANALOG11    = 65;
  public static final int ANALOG12    = 66;
  public static final int ANALOG13    = 67;
  public static final int ANALOG14    = 68;
  public static final int ANALOG15    = 69;
}
