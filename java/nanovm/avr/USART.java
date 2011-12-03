//
//
// nanovm/avr/USART.java
//
// the contents of this file is actually never realy used
// since class2uvm replaces these calls with native calls
//

package nanovm.avr;
import java.io.PrintStream;
import java.lang.String;

public class USART {
    public static final int None = 0;
    public static final int Odd = 2;
    public static final int Even = 3;
    public static final int B2400 = 6, B4800 = 12, B9600 = 24, 
        B14400 = 36, B19200 = 48, B28800 = 72, B38400 = 96, 
        B57600 = 144, B76800 = 192, B115200 = 288, 
        B230400 = 576, B250000 = 625, B500000 = 1250, B1000000 = 2500;

    public native void println(String x);
    public native void println(int x);
    public native void println(char x);
    public native void print(String s);
    public native void print(int i);
    public native void print(char c);
    public native PrintStream format(String format, Object[] args);
    public native int available();
    public native int read();
    public native void init(int baudRateDivideBy400, int stopbit, int parity);
    public void init(int baudRateDivideBy400){ init(baudRateDivideBy400, None, 1); }
    public void init(int baudRateDivideBy400, int stopbit){ init(baudRateDivideBy400, None, stopbit); }
}
