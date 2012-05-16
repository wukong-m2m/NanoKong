#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_GENERIC

extern void wuclass_generic_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_generic_properties[] = {
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READONLY,
};

wkpf_wuclass_definition wuclass_generic = {
  WKPF_WUCLASS_GENERIC, // wuclass id
  wuclass_generic_update, // update function pointer
  1, // Number of properties
  wuclass_generic_properties
};

#endif // ENABLE_WUCLASS_GENERIC
