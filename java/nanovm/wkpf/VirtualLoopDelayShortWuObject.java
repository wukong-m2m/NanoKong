package nanovm.wkpf;


public class VirtualLoopDelayShortWuObject extends GENERATEDVirtualLoopDelayShortWuObject {
    private short delay_count_short;

    public VirtualLoopDelayShortWuObject(){
        delay_count_short=0;
    }

    public void update() {
        // TODONR: replace these calls with convenience methods in VirtualWuObject once we get the inheritance issue sorted out.
        short input = WKPF.getPropertyShort(this, INPUT);
        short delay = WKPF.getPropertyShort(this, DELAY);
        short output = WKPF.getPropertyShort(this, OUTPUT);

        if (delay_count_short>=delay) {
          delay_count_short=0;
	  WKPF.setPropertyShort(this, OUTPUT, input);
          System.out.println("WKPFUPDATE(loop_delay): Native loop_delay: write" + input + "to output \n");
        }
        else if(output!=input) {
	  delay_count_short++;
          System.out.println("WKPFUPDATE(loop_delay): Native loop_delay: delay" + delay + ", now count to " + delay_count_short +"\n");
        }

    }
}
