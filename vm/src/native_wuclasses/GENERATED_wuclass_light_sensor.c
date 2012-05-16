#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_LIGHT_SENSOR

extern void wuclass_light_sensor_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_light_sensor_properties[] = {
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READ, // WKPF_PROPERTY_LIGHT_SENSOR_CURRENT_VALUE
  WKPF_PROPERTY_TYPE_REFRESH_RATE+WKPF_PROPERTY_ACCESS_RW
};

wkpf_wuclass_definition wuclass_light_sensor = {
  WKPF_WUCLASS_LIGHT_SENSOR, // wuclass id
  wuclass_light_sensor_update, // update function pointer
  2, // Number of properties
  wuclass_light_sensor_properties
};

#endif // ENABLE_WUCLASS_LIGHT_SENSOR
