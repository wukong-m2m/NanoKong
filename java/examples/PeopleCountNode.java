import java.io.*;
import nanovm.avr.*;
import nanovm.io.*;

class PeopleCountNode {
    public static void main(String[] args) throws IOException {
        byte [] data = {1,2,3,4,5,6,7,8};
        char c = 0;
        int inMotionPin = 3;
        int inIR1Pin = 4;
        int inIR2Pin = 5;

        int outMotionPin = 7;
        int outIR1Pin = 9;
        int outIR2Pin = 11;
        /* Old way
           AVR.portL.setOutput(0); // Digital Pin 49
           AVR.portL.setBit(0); // Used for testing
           AVR.portH.setInput(5); // Digital Pin 8
           AVR.portE.setInput(5); // Digital Pin 3
           AVR.portE.setInput(4); // Digital Pin 2
           */

        /*AVR.setPinIOMode(inMotionPin, 1);
        AVR.setPinIOMode(inIR1Pin, 1);
        AVR.setPinIOMode(inIR2Pin, 1);
        AVR.setPinIOMode(outMotionPin, 1);
        AVR.setPinIOMode(outIR1Pin, 1);
        AVR.setPinIOMode(outIR2Pin, 1);
        */
        while(true) {
            /*data[0] = (byte)AVR.digitalRead(outMotionPin);
            data[1] = (byte)AVR.digitalRead(outIR1Pin);
            data[2] = (byte)AVR.digitalRead(outIR2Pin);
            data[3] = (byte)AVR.digitalRead(inMotionPin);
            data[4] = (byte)AVR.digitalRead(inIR1Pin);
            data[5] = (byte)AVR.digitalRead(inIR2Pin);*/
            for (int i = 0; i < data.length; ++i)
                data[i] = (byte)((data[i]+1) & (0xFF));
            //    System.out.print(data[i]);
            //System.out.print('\n');
            NvmComm3.send((byte)1, data, (byte)data.length);
            Timer.wait(3000);
        }
    }
}


