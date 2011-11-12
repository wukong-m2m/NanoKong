import java.io.*;
import nanovm.avr.*;

class UsartSender {
    public static void main(String[] args) throws IOException {
        System.out.println("Please input data.....");
        // We have usart[0-3], but 0 is assigned to System.
        // So we have only usart[1-3] to use.
        // Baudrate choices :
        // 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 
        // 76800, 115200, 230400, 250000, 500000, 1000000
        AVR.usart3.setBaudrate(115200);
        AVR.usart3.setParity(AVR.usart3.NoneParity); // None, Odd, and Even parity
        AVR.usart3.setStopbits(2); // either 2 or 1 stopbit
        while(true) {
            if (System.in.available() != 0) {
                char c = (char)System.in.read();
                AVR.usart3.print(c);
            }
        }
    }
}


