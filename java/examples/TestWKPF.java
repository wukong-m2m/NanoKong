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
	  WKPF.registerProfile((short)0x42, properties, (byte)properties.length);
      System.out.println("======="+WKPF.getErrorCode());

      System.out.println("Creating profile instance and endpoint\n");
      VirtualThresholdProfile profileInstanceA = new VirtualThresholdProfile();
	  WKPF.createEndpoint((short)0x42, (byte)0x2, profileInstanceA);
      System.out.println("======="+WKPF.getErrorCode());
      System.out.println("Created profile instance\n");

      System.out.println("Creating profile instance and endpoint\n");
      VirtualThresholdProfile profileInstanceB = new VirtualThresholdProfile();
	  WKPF.createEndpoint((short)0x42, (byte)0x2, profileInstanceB);
      System.out.println("======="+WKPF.getErrorCode());
      System.out.println("Created profile instance\n");

      System.out.println("Removing profile instance and endpoint\n");
	  WKPF.removeEndpoint((byte)0x2);
      System.out.println("======="+WKPF.getErrorCode());
      System.out.println("Removed profile instance\n");

      System.out.println("Creating profile instance and endpoint\n");
	  WKPF.createEndpoint((short)0x42, (byte)0x2, profileInstanceB);
      System.out.println("======="+WKPF.getErrorCode());
      System.out.println("Created profile instance\n");

      System.out.println("WuKong Profile Framework test - done\n");
  }
}
