import java.io.*;
import nanovm.avr.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;

public class DemoTest {

    // =========== Begin: Generated by the translator from application WuML
    /* Component instance IDs to indexes:
    */

    //link table
    // fromInstanceIndex(2 bytes), fromPropertyId(1 byte), toInstanceIndex(2 bytes), toPropertyId(1 byte), toWuClassId(2 bytes)
    //eg. (byte)0,(byte)0, (byte)0, (byte)2,(byte)0, (byte)1, (byte)1,(byte)0
    private final static byte[] linkDefinitions = {
        // Note: Component instance id and wuclass id are little endian
        // Note: using WKPF constants now, but this should be generated as literal bytes by the WuML->Java compiler.
        // Connect input controller to threshold
    };

    //component node id and port number table
    // each row corresponds to the component index mapped from component ID above
    // each row has two items: node id, port number
    private final static byte[][] componentInstanceToWuObjectAddrMap = {
    };
    // =========== End: Generated by the translator from application WuML

    public static void main (String[] args) {
        System.out.println("DemoTest");
        WKPF.loadComponentToWuObjectAddrMap(componentInstanceToWuObjectAddrMap);
        WKPF.loadLinkDefinitions(linkDefinitions);
        initialiseLocalWuObjects();

        while(true){
            VirtualWuObject wuclass = WKPF.select();
            if (wuclass != null) {
                wuclass.update();
            }
        }
    }

    private static void initialiseLocalWuObjects() {

    }
}