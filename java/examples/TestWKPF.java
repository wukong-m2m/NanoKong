/*
  TestWKPF.java
 */

import java.io.*;
import nanovm.wkpf.*;

public class TestWKPF {
  private static final byte[] properties = new byte[] {
        WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
        WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
        WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
        WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_READ};

  public static void main(String[] args) {
      
      
      System.out.println("WuKong Profile Framework test\n");
	  byte retval = WKPF.registerProfile((short)0x42, properties, (byte)properties.length);
      System.out.println(retval);

      System.out.println("Creating profile instance\n");
      VirtualThresholdProfile profileInstance = new VirtualThresholdProfile();
      System.out.println("Created profile instance\n");
//	  retval = WKPF.createEndpoint((short)0x42, (byte)0x2, profileInstance);
      System.out.println(retval);      

      System.out.println("WuKong Profile Framework test - done\n");
  }
}
