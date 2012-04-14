/*
  VirtualTestProfile.java
 */

import java.io.*;
import nanovm.wkpf.*;

public class VirtualTestProfile extends VirtualProfile {
    public static final byte[] properties = new byte[] {
          WKPF.PROPERTY_TYPE_SHORT|WKPF.PROPERTY_ACCESS_RW,
          WKPF.PROPERTY_TYPE_BOOLEAN|WKPF.PROPERTY_ACCESS_RW};
          
    public void update() {
        
    }
}