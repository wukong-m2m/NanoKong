import java.io.*;
import nanovm.avr.*;

class UsartSender {
    public static void main(String[] args) throws IOException {
        System.out.println("This is sender.....");
        // We have usart[0-3], but 0 is assigned to System.
        // So we have only usart[1-3] to use.
        // 1. Baudrate choices :
        //  B2400, B4800, B9600, B14400, B19200, B28800, B38400, B57600, 
        //  B76800, B115200, B230400, B250000, B500000, B1000000
        // 2. 2 or 1 stopbits
        // 3. None, Odd, and Even parity
        AVR.usart3.init(USART.B115200, 2, USART.None);
        // WARNING:
        //   AVR.usart0.init(115200, 1, USART.None);
        // will be set usart port 0 as the output pin but USB port
        // And we cannot set baudrate to be more than 16383
        while(true) {
            if (System.in.available() != 0) {
                char c = (char)System.in.read();
                AVR.usart0.print(c);
            }
        }
    }
}


