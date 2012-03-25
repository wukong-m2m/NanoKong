//
// nanovm/io/Lcd.java
//
// When converting NanoVM code using the Convert tool, this
// code will magically be replaced by native methods. This
// code will never be called.
//
// It is possible to fill this code with e.g. a graphical 
// representation of the Asuro allowing the Java code to be
// run on a PC using a Asuro simulation
//

package nanovm.io;

public class NvmComm {
  // Length must not exceed NVC3_MESSAGE_SIZE, defined in nvmcomm.h
  // A return value of 0 indicates success, -1 failure while sending,
  // -2 is returned if length>NVC3_MESSAGE_SIZE
  public static native int send(byte dest, byte data[], byte length);
  public static native byte[] receive(int waitmsec);
}
