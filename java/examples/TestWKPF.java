/*
  TestWKPF.java
 */

import java.io.*;
import nanovm.wkpf.*;

class TestWKPF {
  private static final byte[] properties = new byte[] {
        WKPF.PROPERTY_TYPE_SHORT&WKPF.PROPERTY_ACCESS_RW,
        WKPF.PROPERTY_TYPE_SHORT&WKPF.PROPERTY_ACCESS_RW,
        WKPF.PROPERTY_TYPE_SHORT&WKPF.PROPERTY_ACCESS_RW,
        WKPF.PROPERTY_TYPE_BOOLEAN&WKPF.PROPERTY_ACCESS_READ};

  public static void main(String[] args) {
      System.out.println("WuKong Profile Framework test\n");
	  System.out.println(WKPF.PROPERTY_TYPE_SHORT);
	  System.out.println(WKPF.PROPERTY_TYPE_BOOLEAN);
	  System.out.println(WKPF.PROPERTY_ACCESS_READ);
	  System.out.println(WKPF.PROPERTY_ACCESS_WRITE);
	  System.out.println(WKPF.PROPERTY_ACCESS_RW);
	  System.out.println(WKPF.PROPERTY_TYPE_SHORT&WKPF.PROPERTY_ACCESS_RW);
	  System.out.println(WKPF.PROPERTY_TYPE_BOOLEAN&WKPF.PROPERTY_ACCESS_WRITE);
	  byte retval = WKPF.registerProfile((short)0x42, properties, (byte)properties.length);
      System.out.println(retval);
      System.out.println("WuKong Profile Framework test - done\n");
  }
}
