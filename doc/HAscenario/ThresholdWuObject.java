import wukong.WKPF;
import wukong.VirtualWuObject;

public class ThresholdWuObject extends VirtualWuObject {
	public static final int OPERATOR_GT = 0;
	public static final int OPERATOR_LT = 1;
	public static final int OPERATOR_GTE = 2;
	public static final int OPERATOR_LTE = 3;

	public static final int WUCLASSDEF_THRESHOLD = 0x8000; // WuClass IDs >= 0x8000 are custom virtual wuclasses

	public static final int WUCLASSDEF_THRESHOLD_OPERATOR = 0;
	public static final int WUCLASSDEF_THRESHOLD_THRESHOLD = 1;
	public static final int WUCLASSDEF_THRESHOLD_VALUE = 2;
	public static final int WUCLASSDEF_THRESHOLD_EXCEEDED = 3;

	public byte[] getDefinition() {
		return {WUCLASSDEF_THRESHOLD,
						4,
						WKPF.PROPERTY_TYPE_INT + WKPF.PROPERTY_ACCESS_RW,
						WKPF.PROPERTY_TYPE_INT + WKPF.PROPERTY_ACCESS_RW,
						WKPF.PROPERTY_TYPE_INT + WKPF.PROPERTY_ACCESS_RW,
						WKPF.PROPERTY_TYPE_BOOLEAN + WKPF.PROPERTY_ACCESS_RO,
						};
	}
	
	public void update() {
		int operator = getPropertyInt(WUCLASSDEF_THRESHOLD_OPERATOR);
		int threshold = getPropertyInt(WUCLASSDEF_THRESHOLD_THRESHOLD);
		int value = getPropertyInt(WUCLASSDEF_THRESHOLD_VALUE);
		
		if (operator == OPERATOR_GT && value > threshold)
			setPropertyBoolean(WUCLASSDEF_THRESHOLD_EXCEEDED, true);
		else if (operator == OPERATOR_LT && value < threshold)
			setPropertyBoolean(WUCLASSDEF_THRESHOLD_EXCEEDED, true);
		else
			setPropertyBoolean(WUCLASSDEF_THRESHOLD_EXCEEDED, false);
	}
}
