import java.io.*;
import nanovm.avr.*;

class UsartSender {
    public static void main(String[] args) throws IOException {
        System.out.println("This is sender.....");
        // We have usart[0-3], but 0 is assigned to System.
        // So we have only usart[1-3] to use.
        // 1. Baudrate choices :
        //  2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 
        //  76800, 115200, 230400, 250000, 500000, 1000000
        // 2. 2 or 1 stopbits
        // 3. None, Odd, and Even parity
        AVR.usart3.init(9600, 2, USART.None);
        // WARNING:
        //   AVR.usart0.init(115200, 1, USART.None);
        // will be set usart port 0 as the output pin but USB port
        // And we cannot set baudrate to be more than 16383
        while(true) {
            if (System.in.available() != 0) {
                char c = (char)System.in.read();
                AVR.usart3.print(c);
            }
        }
    }
}


