import nanovm.avr.*;
import nanovm.io.*;

class ZZLightController {
    private static final byte COMMAND_SET_LAMP = 1;
    private static final short COMMAND_SET_LAMP_ACK = 11;

    private static final byte ROUTER_ID = 64; 

    public static void main(String[] args) {
        AVR.setPinIOMode(AVR.DIGITAL32, AVR.OUTPUT);
        AVR.digitalWrite(AVR.DIGITAL32, 0);
        System.out.println("This is Controller speaking.....");
        while(true) {
            byte[] data = NvmComm3.receive(1000);
            if (data != null && data.length == 3 && data[1] == COMMAND_SET_LAMP) {
                if (data[2] == 0) {
                    AVR.digitalWrite(AVR.DIGITAL32, 0);
                    System.out.println("Light OFF");
                }
                else {
                    AVR.digitalWrite(AVR.DIGITAL32, 1);
                    System.out.println("Light ON");
                }
                NvmComm3.send(ROUTER_ID, new byte[]{data[0], COMMAND_SET_LAMP_ACK}, (byte)2);
            }
        }
    }
}
