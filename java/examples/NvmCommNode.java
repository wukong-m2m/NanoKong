import nanovm.avr.*;
import nanovm.io.*;

class NvmCommNode {
    public static void main(String[] args) {
        byte [] data = {1,2,3,4,5,6,7,8};
        char c = 0;
        /* Old way
           AVR.portL.setOutput(0); // Digital Pin 49
           AVR.portL.setBit(0); // Used for testing
           AVR.portH.setInput(5); // Digital Pin 8
           */

        /* New way
           AVR.setPinIOMode(49, AVR.OUTPUT);
           AVR.digitalWrite(49, 1);
           AVR.setPinIOMode(8, AVR.INPUT);
           */
        while(true) {
            /* New Way
               data[0] = (byte)AVR.digitalRead(8);
              */
            for (int i = 0; i < data.length; ++i){
                data[i] = (byte)((data[i]+2) & (0xFF));
                System.out.print(data[i]);
            }
            System.out.print('\n');
            NvmComm.send((byte)64, data, (byte)data.length); // 0~64 is Zwave 0~64; 64~127 is Zigbee 0~63
            Timer.wait(1000);
        }
    }
}


