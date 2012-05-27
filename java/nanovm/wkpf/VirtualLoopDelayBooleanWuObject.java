package nanovm.wkpf;


public class VirtualLoopDelayBooleanWuObject extends GENERATEDVirtualLoopDelayBooleanWuObject {
    private short delay_count_boolean;

    public VirtualLoopDelayBooleanWuObject(){
        delay_count_boolean=0;
    }

    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        boolean input = WKPF.getPropertyBoolean(this, INPUT);
        short delay = WKPF.getPropertyShort(this, DELAY);
        boolean output = WKPF.getPropertyBoolean(this, OUTPUT);

        if (delay_count_boolean>=delay) {
          delay_count_boolean=0;
	  WKPF.setPropertyBoolean(this, OUTPUT, input);
          System.out.println("WKPFUPDATE(loop_delay): Native loop_delay: write" + input + "to output \n");
        }
        else if(output!=input) {
	  delay_count_boolean++;
          System.out.println("WKPFUPDATE(loop_delay): Native loop_delay: delay" + delay + ", now count to " + delay_count_boolean +"\n");
        }

    }
}
