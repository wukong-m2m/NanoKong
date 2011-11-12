//
//
// nanovm/avr/USART.java
//
// the contents of this file is actually never realy used
// since class2uvm replaces these calls with native calls
//

package nanovm.avr;
import java.io.PrintStream;
import java.lang.*;

public class USART {
    public static final int NoneParity = 0;
    public static final int OddParity = 2;
    public static final int EvenParity = 3;

    public native void println(String x);
    public native void println(int x);
    public native void println(char x);
    public native void print(String s);
    public native void print(int i);
    public native void print(char c);
    public native PrintStream format(String format, Object[] args);
    public native int available();
    public native int read();
    public native void setBaudrate(int baudrate);
    public native void setParity(int parity);
    public native void setStopbits(int stopbit);
}
