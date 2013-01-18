package nanovm.wkpf;

public class VirtualNullWuObject extends GENERATEDVirtualNullWuObject {
    public void update() {
        // Force property to be sent
        WKPF.setPropertyShort(this, NULL, (short)0);
    }
}
