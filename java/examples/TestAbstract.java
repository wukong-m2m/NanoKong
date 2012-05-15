/*
TestWKPF.java
*/

import java.io.*;
import nanovm.wkpf.*;

public class TestAbstract {
  public static void main(String args[]) {
    System.out.println("before new VirtualProfile()");
    VirtualProfile p = new VirtualThresholdProfile();
    System.out.println("after new VirtualProfile()");
    p.update();
    System.out.println("Abstract works with superclass type");
  }
}
