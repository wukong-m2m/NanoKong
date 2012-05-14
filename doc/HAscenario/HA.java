import wukong.WKPF;
import wukong.VirtualWuObject;

/* Source WuML:
<application name="HAScenario">
	<service type="NumericController" instanceId="ThermostatController1">
		<link fromSlot="out" toInstance="Threshold1" toSlot="threshold" />		
	</service>
	<service type="TemperatureSensor" instanceId="TemparatureSensor1">
		<link fromSlot="currentTemperature" toInstanceId="Threshold1" toSlot="value">
			<filter type="minChange" value="2.0"
		</link>
	</service>
	<service type="Threshold" instanceId="Threshold1">
		<property name="operator" value="<"/>
		<link fromSlot="out" toInstanceId="Heater1" toSlot="onOff" />
	</service>
	<service type="Heater" instanceId="Heater1" />
</application>


Discovered WuClasses:
 - Thermostat controller wuclass at node 0x1234, wuobject 0
 - 2 Temperature sensors wuclass at node 0x1212, wuobjects 3 and 4
 - Heater wuclass wuobject at node 0x2299

The compiler decided to use the temperature sensor at wuobject 4,
 and instantiate a threshold wuclass at node 0x2299, wuobject 1
*/



public class HA {
  private final int COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1 = 0;
  private final int COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1 = 1;
  private final int COMPONENT_INSTANCE_ID_THRESHOLD1 = 2;
  private final int COMPONENT_INSTANCE_ID_LIGHT1 = 3;
  
  private final WuObject[] componentInstanceToWuObjectAddrMapping = { // Indexed by component instance id.
    new WuObjectId(0x1234, 0), // Thermostat controller at node 0, port 0
    new WuObjectId(0x1212, 4), // Temperature sensor at node 1, port 4
    new WuObjectId(0x2299, 1), // Threshold at node 2, port 1
    new WuObjectId(0x2299, 0), // Heater at node 2, port 0
  }
  
  public static EndPointId ComponentInstancetoWuObject(int componentInstanceId) {
    // INITIAL STATIC VERSION: This could later be replaced by something more dynamic, for now the table is a hardcoded constant
    return wuobjectToComponentInstanceMapping[componentInstanceId];
  }

  public static void main(String[] args) {
    if (WKPF.getMyNodeId() == 1) {
      WKPF.setPropertyInt(ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1), WKPF.WUCLASSDEF_GENERAL_REFRESHRATE, 5000); // Sample the temperature every 5 seconds
    }
    if (WKPF.getMyNodeId() == 2) {
      WKPF.addWuClass(new ThresholdWuObject(), 1); // Instantiate the threshold wuclass. INITIAL STATIC VERSION: This should later be replaced by return value from WKPF.wait so the framework can dynamically allocate a new wuclass
      WKPF.setPropertyInt(ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_THRESHOLD1), ThresholdWuObject.WUCLASSDEF_THRESHOLD_OPERATOR, ThresholdWuObject.OPERATOR_LT); // Sample the temperature every 5 seconds
    }

    while(true) {
      VirtualWuObject wuclass = WKPF.wait();
      if (wuclass != null) {
        wuclass.update();
      }
      propertyDispatch();
    }
  }

  public static void propertyDispatch() {
    WKPF.DirtyProperty dirtyProperty = null;
    
    while (dirtyProperty = WKPF.getNextDirtyProperty()) {
      if (ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1) == dirtyProperty.wuobjectId
          && dirtyProperty.propertyId == WKPF.WUCLASSDEF_TEMPERATURESENSOR_CURRENTTEMPERATURE) {
        int lastPropagatedValue;
        if (Math.Abs(lastPropagatedValue - dirtyProperty.valueInt) > 2) {
          lastPropagatedValue = dirtyProperty.valueInt;
          WKPF.setPropertyInt(ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_THRESHOLD1), WKPF.WUCLASSDEF_THRESHOLD_VALUE, dirtyProperty.valueInt);
        }
      } else if (ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1) == dirtyProperty.wuobjectId
                 && dirtyProperty.propertyId == WKPF.WUCLASSDEF_NUMERICCONTROLLER_OUT) {
          WKPF.setPropertyInt(ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_THRESHOLD1), WKPF.WUCLASSDEF_THRESHOLD_THRESHOLD, dirtyProperty.valueInt);
      } else if (ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_THRESHOLD1) == dirtyProperty.wuobjectId
                 && dirtyProperty.propertyId == WKPF.WUCLASSDEF_THRESHOLD_EXCEEDED) {
          WKPF.setPropertyBoolean(ComponentInstancetoWuObject(COMPONENT_INSTANCE_ID_LIGHT1), WKPF.WUCLASSDEF_LIGHT_ONOFF, dirtyProperty.valueBoolean);
      }
    }
  }
}
