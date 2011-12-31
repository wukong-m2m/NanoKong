import nanovm.avr.*;
import nanovm.io.*;

class IntelDemoLightSensorV1 {
  private static final short COMMAND_SET_LAMP = 1;
  private static final short COMMAND_SET_THRESHOLD = 2;

  private static final byte LAMP_NODE_ID = 3;

  private static final short INITIAL_THRESHOLD = 127;

  private static void setLight(boolean turn_on) {
    if(turn_on) {
      NvmComm3.send(LAMP_NODE_ID, new byte[] {COMMAND_SET_LAMP, 1}, (byte)2);
      System.out.println("Sending 'lamp on' command");
    }
    else {
      NvmComm3.send(LAMP_NODE_ID, new byte[] {COMMAND_SET_LAMP, 0}, (byte)2);
      System.out.println("Sending 'lamp off' command");
    }
  }

  public static void main(String[] args) {
    int threshold = INITIAL_THRESHOLD;
    boolean lamp_on = false;

    Adc.setPrescaler(Adc.DIV64);
    Adc.setReference(Adc.INTERNAL);

    while(true) {
      byte[] data = NvmComm3.receive(1000);
        if (data != null && data.length == 2 && data[0] == COMMAND_SET_THRESHOLD) {
            threshold = 0xFF & data[1]; // necessary since Java doesn't have unsigned bytes
            System.out.println("Set threshold to " + threshold);
        }
        int light_sensor_reading = Adc.getByte(Adc.CHANNEL0);
        System.out.println("Sensed value:" + light_sensor_reading);
        System.out.println("Threshold:" + threshold);
        if (lamp_on)
          System.out.println("Lamp: On");
        else
          System.out.println("Lamp: Off");
        if (light_sensor_reading <= threshold && !lamp_on) {
            setLight(true);
            lamp_on = true;
        }
        if (light_sensor_reading > threshold && lamp_on) {
            setLight(false);
            lamp_on = false;
        }
    }
  }
}
