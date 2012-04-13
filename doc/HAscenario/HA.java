import wukong.WKPF;
import wukong.VirtualProfile;

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


Discovered profiles:
 - Thermostat controller profile at node 0x1234, endpoint 0
 - 2 Temperature sensors profile at node 0x1212, endpoints 3 and 4
 - Heater profile endpoint at node 0x2299

The compiler decided to use the temperature sensor at endpoint 4,
 and instantiate a threshold profile at node 0x2299, endpoint 1
*/



public class HA {
  private final int COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1 = 0;
  private final int COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1 = 1;
  private final int COMPONENT_INSTANCE_ID_THRESHOLD1 = 2;
  private final int COMPONENT_INSTANCE_ID_HEATER1 = 3;
	
	private final Endpoint[] componentInstanceToEndpointMapping = { // Indexed by component instance id.
		new EndpointId(0x1234, 0), // Thermostat controller at node 0, port 0
		new EndpointId(0x1212, 4), // Temperature sensor at node 1, port 4
		new EndpointId(0x2299, 1), // Threshold at node 2, port 1
		new EndpointId(0x2299, 0), // Heater at node 2, port 0
	}
	
	public static EndPointId ComponentInstancetoEndpoint(int componentInstanceId) {
		// INITIAL STATIC VERSION: This could later be replaced by something more dynamic, for now the table is a hardcoded constant
		return endpointToComponentInstanceMapping[componentInstanceId];
	}

  public static void main(String[] args) {
		if (WKPF.getMyNodeId() == 1) {
			WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1), WKPF.PROFILEDEF_GENERAL_REFRESHRATE, 5000); // Sample the temperature every 5 seconds
		}
		if (WKPF.getMyNodeId() == 2) {
			WKPF.addProfile(new ThresholdProfile(), 1); // Instantiate the threshold profile. INITIAL STATIC VERSION: This should later be replaced by return value from WKPF.wait so the framework can dynamically allocate a new profile
			WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1), ThresholdProfile.PROFILEDEF_THRESHOLD_OPERATOR, ThresholdProfile.OPERATOR_LT); // Sample the temperature every 5 seconds
		}

		while(true) {
			VirtualProfile profile = WKPF.wait();
			if (profile != null) {
				profile.update();
			}
			propertyDispatch();
		}
	}

	public static void propertyDispatch() {
		WKPF.DirtyProperty dirtyProperty = null;
		
		while (dirtyProperty = WKPF.getNextDirtyProperty()) {
			if (ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1) == dirtyProperty.endpointId
					&& dirtyProperty.propertyId == WKPF.PROFILEDEF_TEMPERATURESENSOR_CURRENTTEMPERATURE) {
				int lastPropagatedValue;
				if (Math.Abs(lastPropagatedValue - dirtyProperty.valueInt) > 2) {
					lastPropagatedValue = dirtyProperty.valueInt;
					WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1), WKPF.PROFILEDEF_THRESHOLD_VALUE, dirtyProperty.valueInt);
				}
			} else if (ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1) == dirtyProperty.endpointId
								 && dirtyProperty.propertyId == WKPF.PROFILEDEF_NUMERICCONTROLLER_OUT) {
					WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1), WKPF.PROFILEDEF_THRESHOLD_THRESHOLD, dirtyProperty.valueInt);
			} else if (ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1) == dirtyProperty.endpointId
								 && dirtyProperty.propertyId == WKPF.PROFILEDEF_THRESHOLD_EXCEEDED) {
					WKPF.setPropertyBoolean(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_HEATER1), WKPF.PROFILEDEF_HEATER_ONOFF, dirtyProperty.valueBoolean);
			}
		}
	}
}
