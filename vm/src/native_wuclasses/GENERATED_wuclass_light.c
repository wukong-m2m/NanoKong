#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_LIGHT

extern void wuclass_light_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_light_properties[] = {
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READWRITE // WKPF_PROPERTY_LIGHT_ONOFF
};

wkpf_wuclass_definition wuclass_light = {
  WKPF_WUCLASS_LIGHT, // wuclass id
  wuclass_light_update, // update function pointer
  1, // Number of properties
  wuclass_light_properties
};

#endif // ENABLE_WUCLASS_LIGHT
