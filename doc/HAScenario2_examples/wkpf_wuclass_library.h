// Common to all wuclasses
#define WKPF_PROPERTY_COMMON_REFRESHRATE                          0
#define WKPF_PROP_BASE                                            0 // TODO: implement this

#define WKPF_WUCLASS_GENERIC                                      0
#define WKPF_PROPERTY_GENERIC_DUMMY                               (WKPF_PROP_BASE+0)

#define WKPF_WUCLASS_THRESHOLD                                    1
#define WKPF_PROPERTY_THRESHOLD_OPERATOR                          (WKPF_PROP_BASE+0)
#define WKPF_PROPERTY_THRESHOLD_THRESHOLD                         (WKPF_PROP_BASE+1)
#define WKPF_PROPERTY_THRESHOLD_VALUE                             (WKPF_PROP_BASE+2)
#define WKPF_PROPERTY_THRESHOLD_OUTPUT                            (WKPF_PROP_BASE+3)

#define WKPF_WUCLASS_TEMPERATURE_SENSOR                           2
#define WKPF_PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE      (WKPF_PROP_BASE+0)

#define WKPF_WUCLASS_NUMERIC_CONTROLLER                           3
#define WKPF_PROPERTY_NUMERIC_CONTROLLER_OUTPUT                   (WKPF_PROP_BASE+0)

#define WKPF_WUCLASS_LIGHT                                        4
#define WKPF_PROPERTY_LIGHT_ONOFF                                 (WKPF_PROP_BASE+0)

#define WKPF_WUCLASS_LIGHT_SENSOR                                 5
#define WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE                  (WKPF_PROP_BASE+0)

#define WKPF_AND_GATE                                             6
#define WKPF_PROPERTY_AND_GATE_IN1                                (WKPF_PROP_BASE+0)
#define WKPF_PROPERTY_AND_GATE_IN2                                (WKPF_PROP_BASE+1)
#define WKPF_PROPERTY_AND_GATE_OUTPUT                             (WKPF_PROP_BASE+2)
