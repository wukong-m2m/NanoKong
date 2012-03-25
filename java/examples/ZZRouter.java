import nanovm.avr.*;
import nanovm.io.*;

class ZZRouter {
    public static void main(String[] args) {
        System.out.println("This is Router speaking.....");
        while(true) {
            byte[] data = NvmComm.receive(500);
            if (data != null) {
                if (data.length == 4) {
                    System.out.println("From: "+data[0]+", To: "+data[1]+", Cmd: "+data[2]+", Val: "+data[3]);
                    NvmComm.send(data[1], new byte[] {data[0], data[2], data[3]}, (byte)3);
                }
                else if (data.length == 2) {
                    System.out.println("To: "+data[0]+", Cmd: "+data[1]);
                    NvmComm.send(data[0], new byte[] {data[1]}, (byte)1);
                }else{
                    System.out.println("Invalid msg");
                }
            }
        }
    }
}
