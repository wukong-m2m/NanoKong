import wukong.WKPF;
import wukong.VirtualProfile;

public class ThresholdProfile extends VirtualProfile {
	public static final int OPERATOR_GT = 0;
	public static final int OPERATOR_LT = 1;

	public static final int PROFILEDEF_THRESHOLD = 0x8000; // Profile IDs >= 0x8000 are custom virtual profiles

	public static final int PROFILEDEF_THRESHOLD_OPERATOR = 0;
	public static final int PROFILEDEF_THRESHOLD_THRESHOLD = 1;
	public static final int PROFILEDEF_THRESHOLD_VALUE = 2;
	public static final int PROFILEDEF_THRESHOLD_EXCEEDED = 3;

	public byte[] getDefinition() {
		return {PROFILEDEF_THRESHOLD,
						4,
						WKPF.PROPERTY_TYPE_INT + WKPF.PROPERTY_ACCESS_RW,
						WKPF.PROPERTY_TYPE_INT + WKPF.PROPERTY_ACCESS_RW,
						WKPF.PROPERTY_TYPE_INT + WKPF.PROPERTY_ACCESS_RW,
						WKPF.PROPERTY_TYPE_BOOLEAN + WKPF.PROPERTY_ACCESS_RO,
						};
	}
	
	public void update() {
		int operator = getPropertyInt(PROFILEDEF_THRESHOLD_OPERATOR);
		int threshold = getPropertyInt(PROFILEDEF_THRESHOLD_THRESHOLD);
		int value = getPropertyInt(PROFILEDEF_THRESHOLD_VALUE);
		
		if (operator == OPERATOR_GT && value > threshold)
			setPropertyBoolean(PROFILEDEF_THRESHOLD_EXCEEDED, true);
		else if (operator == OPERATOR_LT && value < threshold)
			setPropertyBoolean(PROFILEDEF_THRESHOLD_EXCEEDED, true);
		else
			setPropertyBoolean(PROFILEDEF_THRESHOLD_EXCEEDED, false);
	}
}
