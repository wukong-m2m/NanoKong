import java.io.*;
import nanovm.wkpf.*;

public class HAScenario {    
  private static final int COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1 = 0;
  private static final int COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1 = 1;
  private static final int COMPONENT_INSTANCE_ID_THRESHOLD1 = 2;
  private static final int COMPONENT_INSTANCE_ID_HEATER1 = 3;
  
  private static int myNodeId;
  private static int lastPropagatedValue;
  
  private static final Endpoint[] componentInstanceToEndpointMapping = { // Indexed by component instance id.
    new Endpoint(77, 1), // Thermostat
    new Endpoint(77, 2), // Temperature sensor
    new Endpoint(77, 3), // Threshold
    new Endpoint(77, 4), // Heater
  }
  
  public static EndPointId ComponentInstancetoEndpoint(int componentInstanceId) {
    // INITIAL STATIC VERSION: This could later be replaced by something more dynamic, for now the table is a hardcoded constant
    return endpointToComponentInstanceMapping[componentInstanceId];
  }

  public static void main(String[] args) {
      myNodeId = WKPF.getMyNodeId();

/*    if (myNodeId == 1) {
      WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1), WKPF.PROFILEDEF_GENERAL_REFRESHRATE, 5000); // Sample the temperature every 5 seconds
    }
    if (myNodeId == 2) {
      WKPF.addProfile(new ThresholdProfile(), 1); // Instantiate the threshold profile. INITIAL STATIC VERSION: This should later be replaced by return value from WKPF.wait so the framework can dynamically allocate a new profile
      WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1), ThresholdProfile.PROFILEDEF_THRESHOLD_OPERATOR, ThresholdProfile.OPERATOR_LT); // Sample the temperature every 5 seconds
    }*/


    while(true) {
      VirtualProfile profile = WKPF.select();
      if (profile != null) {
        profile.update();
      }
      propertyDispatch();
    }
  }

  private static boolean match(short dirtyProperty, int componentInstanceId, byte portNumber) {
    int portNumber = (short)(dirtyProperty >> 8) & (short)0xFF;
    int dirtyPropertyNumber = dirtyProperty & (short)0xFF;  
    Endpoint endpoint = ComponentInstancetoEndpoint(componentInstanceId);

    return endpoint.nodeId == myNodeId
        && endpoint.portNumber == portNumber
        && portNumber == dirtyPortNumber;
  }
  private static void setPropertyShort(int componentInstanceId, byte propertyNumber, short value) {      
      Endpoint endpoint = ComponentInstancetoEndpoint(componentInstanceId);
      WKPF.setPropertyShort(endpoint.nodeId, endpoint.portNumber, propertyNumber,value);
  }
  private static void setPropertyBoolean(int componentInstanceId, byte propertyNumber, boolean value) {
      Endpoint endpoint = ComponentInstancetoEndpoint(componentInstanceId);
      WKPF.setPropertyBoolean(endpoint.nodeId, endpoint.portNumber, propertyNumber,value);
  }

  public static void propertyDispatch() {
    short dirtyProperty = null;

    while((dirtyProperty = WKPF.getNextDirtyProperty()) != 0) {
      if (Match(dirtyProperty, COMPONENT_INSTANCE_ID_TEMPARATURESENSOR1, WKPF.PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE)) {
        if (Math.Abs(lastPropagatedValue - dirtyProperty.valueInt) > 2) {
          lastPropagatedValue = dirtyProperty.valueInt;
          setPropertyInt(COMPONENT_INSTANCE_ID_THRESHOLD1, WKPF.PROFILEDEF_THRESHOLD_VALUE, dirtyProperty.valueInt);
        }
      } else if (Match(dirtyProperty, COMPONENT_INSTANCE_ID_THERMOSTATCONTROLLER1, WKPF.PROFILEDEF_NUMERICCONTROLLER_OUT)) {
          WKPF.setPropertyInt(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_THRESHOLD1), WKPF.PROFILEDEF_THRESHOLD_THRESHOLD, dirtyProperty.valueInt);
      } else if (Match(dirtyProperty, COMPONENT_INSTANCE_ID_THRESHOLD1, WKPF.PROFILEDEF_THRESHOLD_EXCEEDED)) {
          WKPF.setPropertyBoolean(ComponentInstancetoEndpoint(COMPONENT_INSTANCE_ID_HEATER1), WKPF.PROFILEDEF_HEATER_ONOFF, dirtyProperty.valueBoolean);
      }
    }
  }
}
