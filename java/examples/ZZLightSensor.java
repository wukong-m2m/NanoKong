import nanovm.avr.*;
import nanovm.io.*;

class ZZLightSensor {
    private static final short COMMAND_SET_LAMP = 1;

    private static final short COMMAND_SET_LAMP_ACK = 11;

    private static final byte MY_ID = 3;
    private static final byte ROUTER_ID = 1;
    private static final byte LAMP_NODE_ID = 65;

    private static final short INITIAL_THRESHOLD = 240;

    private static boolean setLight(boolean turn_on) {
        if(turn_on) {
            NvmComm.send(ROUTER_ID, new byte[] {MY_ID, LAMP_NODE_ID, COMMAND_SET_LAMP, 1}, (byte)4);
            System.out.println("Sending 'lamp on' command");
        }
        else {
            NvmComm.send(ROUTER_ID, new byte[] {MY_ID, LAMP_NODE_ID, COMMAND_SET_LAMP, 0}, (byte)4);
            System.out.println("Sending 'lamp off' command");
        }
        byte[] data = NvmComm.receive(1000);
        if (data != null && data.length == 1 && data[0] == COMMAND_SET_LAMP_ACK){
            System.out.println("ACK received");
            return true;
        }
        else
            return false;
    }

    public static void main(String[] args) {
        int threshold = INITIAL_THRESHOLD;
        int light_sensor_reading = 255;
        int lamp_on = -1;

        Adc.setPrescaler(Adc.DIV64);
        Adc.setReference(Adc.INTERNAL);
        System.out.println("This is Sensor speaking...");

        while(true) {
            light_sensor_reading = Adc.getByte(Adc.CHANNEL15);
            System.out.println("Sensed value:" + light_sensor_reading);
            System.out.println("Threshold:" + threshold);

            if (lamp_on==1)
                System.out.println("Lamp: On");
            else if (lamp_on==0)
                System.out.println("Lamp: Off");

            if (light_sensor_reading <= threshold && lamp_on != 1) {
                if (setLight(true))
                    lamp_on = 1;
            }
            if (light_sensor_reading > threshold && lamp_on != 0) {
                if (setLight(false))
                    lamp_on = 0;
            }
        }
    }
}
