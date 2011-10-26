import java.io.*;
import nanovm.avr.*;
import nanovm.io.*;

class TestCodeUpdate {
  // private static void Rot13(byte[] data) {
  //   for (int i = 0; i < data.length; i++) {
  //     byte c = data[i];
  //     if      (c >= 'a' && c <= 'm') c += 13;
  //     else if (c >= 'n' && c <= 'z') c -= 13;
  //     else if (c >= 'A' && c <= 'M') c += 13;
  //     else if (c >= 'N' && c <= 'Z') c -= 13;
  //     data[i] = c;
  //   }
  // }
  // 
  // public static void main(String[] args) throws IOException {
  //   System.out.println("NanoVM - TestCodeUpdate");
  //   while(true) {
  //     byte[] data = NvmComm3.receive(1000);
  //     if (data != null) {
  //       Rot13(data);
  //       NvmComm3.send(2, data);
  //     }
  //     System.out.println("BBBBBBBBBBBBBBBBBBBBB");
  //   }
  // }

  public static void main(String[] args) throws IOException {
    byte[] data = {0x40, 0x41, 0x42, 0x43, 0x44, 0x45};
    NvmComm3.send((byte)2, data, (byte)4);
  }
}

     
