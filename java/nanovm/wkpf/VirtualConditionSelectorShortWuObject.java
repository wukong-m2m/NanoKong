package nanovm.wkpf;

public class VirtualConditionSelectorShortWuObject extends GENERATEDVirtualConditionSelectorShortWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short input = WKPF.getPropertyShort(this, INPUT);
        boolean control = WKPF.getPropertyBoolean(this, CONTROL);

        if (control==false) {
	  WKPF.setPropertyShort(this, OUTPUT1, input);
	  WKPF.setPropertyShort(this, OUTPUT2, (short)(0));
          System.out.println("WKPFUPDATE(condition_selector_short): input " + input + " control " + control);
        }
        else {
	  WKPF.setPropertyShort(this, OUTPUT1, (short)(0));
	  WKPF.setPropertyShort(this, OUTPUT2, input);
          System.out.println("WKPFUPDATE(condition_selector_short): input " + input + " control " + control);
        }

    }
}
