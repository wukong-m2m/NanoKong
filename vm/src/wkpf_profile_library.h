// Common to all profiles
#define WKPF_PROPERTY_COMMON_REFRESHRATE                          0
#define WKPF_PROP_BASE                                            0 // TODO: implement this

#define WKPF_PROFILE_GENERIC                                      0
#define WKPF_PROPERTY_GENERIC_DUMMY                               (WKPF_PROP_BASE+0)

#define WKPF_PROFILE_THRESHOLD                                    1
#define WKPF_PROPERTY_THRESHOLD_OPERATOR                          (WKPF_PROP_BASE+0)
#define WKPF_PROPERTY_THRESHOLD_THRESHOLD                         (WKPF_PROP_BASE+1)
#define WKPF_PROPERTY_THRESHOLD_VALUE                             (WKPF_PROP_BASE+2)
#define WKPF_PROPERTY_THRESHOLD_OUTPUT                            (WKPF_PROP_BASE+3)

#define WKPF_PROFILE_TEMPERATURE_SENSOR                           2
#define WKPF_PROPERTY_TEMPERATURE_SENSOR_CURRENT_TEMPERATURE      (WKPF_PROP_BASE+0)

#define WKPF_PROFILE_NUMERIC_CONTROLER                            3
#define WKPF_PROPERTY_NUMERIC_CONTROLER_OUTPUT                    (WKPF_PROP_BASE+0)

#define WKPF_PROFILE_HEATER                                       4
#define WKPF_PROPERTY_HEATER_ONOFF                                (WKPF_PROP_BASE+0)
