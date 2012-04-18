import java.io.*;
import nanovm.wkpf.*;
import nanovm.lang.Math;

public class HAScenario {    
  private static final int COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1 = 0;
  private static final int COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1 = 1;
  private static final int COMPONENT_INSTANCE_ID_THRESHOLD1 = 2;
  private static final int COMPONENT_INSTANCE_ID_HEATER1 = 3;
  
  private static int myNodeId;
  private static int lastPropagatedValue;
  
  private static final Endpoint[] componentInstanceToEndpointMapping = { // Indexed by component instance id.
    new Endpoint((byte)77, (byte)1, WKPF.PROFILE_NUMERIC_CONTROLLER), // Thermostat
    new Endpoint((byte)77, (byte)2, WKPF.PROFILE_TEMPERATURE_SENSOR), // Temperature sensor
    new Endpoint((byte)77, (byte)3, WKPF.PROFILE_THRESHOLD), // Threshold
    new Endpoint((byte)77, (byte)4, WKPF.PROFILE_HEATER), // Heater
  };
  
  public static Endpoint ComponentInstancetoEndpoint(int componentInstanceId) {
    // INITIAL STATIC VERSION: This could later be replaced by something more dynamic, for now the table is a hardcoded constant
    return componentInstanceToEndpointMapping[componentInstanceId];
  }

  private static boolean matchDirtyProperty(int componentInstanceId, byte propertyNumber) {
    Endpoint endpoint = ComponentInstancetoEndpoint(componentInstanceId);
    return endpoint.nodeId == myNodeId
        && endpoint.portNumber == WKPF.getDirtyPropertyPortNumber()
        && propertyNumber == WKPF.getDirtyPropertyNumber();
  }
  private static void setPropertyShort(int componentInstanceId, byte propertyNumber, short value) {      
      Endpoint endpoint = ComponentInstancetoEndpoint(componentInstanceId);
      WKPF.setPropertyShort(endpoint.nodeId, endpoint.portNumber, propertyNumber, endpoint.profileId, value);
  }
  private static void setPropertyBoolean(int componentInstanceId, byte propertyNumber, boolean value) {
      Endpoint endpoint = ComponentInstancetoEndpoint(componentInstanceId);
      WKPF.setPropertyBoolean(endpoint.nodeId, endpoint.portNumber, propertyNumber, endpoint.profileId, value);
  }

  public static void main(String[] args) {
    myNodeId = WKPF.getMyNodeId();

    // ----- REGISTER VIRTUAL PROFILES -----
    // Won't work now since we already have a native profile
    // WKPF.registerProfile((short)WKPF.PROFILE_THRESHOLD, VirtualThresholdProfile.properties, (byte)VirtualThresholdProfile.properties.length);


    // ----- INIT -----
    // INITIAL STATIC VERSION: This should later be replaced by return value from WKPF.wait so the framework can dynamically allocate a new profile
    // Setup the temperature sensor
    if (ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1).nodeId == myNodeId) { 
      setPropertyShort(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1, WKPF.PROPERTY_COMMON_REFRESHRATE, (short)5000); // Sample the temperature every 5 seconds
    }
    // Create and setup the virtual threshold
    if (ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1).nodeId == myNodeId) {
      VirtualProfile profileInstanceThreshold = new VirtualThresholdProfile();
      WKPF.createEndpoint((short)WKPF.PROFILE_THRESHOLD, ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1).portNumber, profileInstanceThreshold);
      setPropertyShort(COMPONENT_INSTANCE_ID_THRESHOLD1, WKPF.PROPERTY_THRESHOLD_OPERATOR, VirtualThresholdProfile.OPERATOR_LT); // Sample the temperature every 5 seconds
    }

    // ----- MAIN LOOP -----
    while(true) {
      VirtualProfile profile = WKPF.select();
      if (profile != null) {
        profile.update();
      }
      propertyDispatch();
    }
  }

  public static void propertyDispatch() {
    while(WKPF.loadNextDirtyProperty()) {

      if (matchDirtyProperty(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1, WKPF.PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE)) {
        short value = WKPF.getDirtyPropertyShortValue();
        if (Math.abs(lastPropagatedValue - value) > 2) {
          lastPropagatedValue = value;
          setPropertyShort(COMPONENT_INSTANCE_ID_THRESHOLD1, WKPF.PROPERTY_THRESHOLD_VALUE, value);
        }

      } else if (matchDirtyProperty(COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1, WKPF.PROPERTY_NUMERIC_CONTROLLER_OUTPUT)) {
        setPropertyShort(COMPONENT_INSTANCE_ID_THRESHOLD1, WKPF.PROPERTY_THRESHOLD_THRESHOLD, WKPF.getDirtyPropertyShortValue());

      } else if (matchDirtyProperty(COMPONENT_INSTANCE_ID_THRESHOLD1, WKPF.PROPERTY_THRESHOLD_OUTPUT)) {
        setPropertyBoolean(COMPONENT_INSTANCE_ID_HEATER1, WKPF.PROPERTY_HEATER_ONOFF, WKPF.getDirtyPropertyBooleanValue());
      }
    }
  }
}
