package nanovm.wkpf;

public class VirtualConditionSelectorBooleanWuObject extends GENERATEDVirtualConditionSelectorBooleanWuObject {
    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        boolean input = WKPF.getPropertyBoolean(this, INPUT);
        boolean control = WKPF.getPropertyBoolean(this, CONTROL);

        if (control==false) {
	  WKPF.setPropertyBoolean(this, OUTPUT1, input);
	  WKPF.setPropertyBoolean(this, OUTPUT2, false);
          //System.out.println("WKPFUPDATE(condition_selector_boolean): input " + input + " control " + control);
        }
        else {
	  WKPF.setPropertyBoolean(this, OUTPUT1, false);
	  WKPF.setPropertyBoolean(this, OUTPUT2, input);
          //System.out.println("WKPFUPDATE(condition_selector_boolean): input " + input + " control " + control);
        }

    }
}
