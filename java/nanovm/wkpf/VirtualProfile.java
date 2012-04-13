package nanovm.wkpf;

public abstract class VirtualProfile {
    public abstract void update();
    
    public native boolean getPropertyBoolean(byte propertyId);
    public native int getPropertyShort(byte propertyId);
    public native void setPropertyBoolean(byte propertyId, boolean value);
    public native void setPropertyShort(byte propertyId, short value);
}
