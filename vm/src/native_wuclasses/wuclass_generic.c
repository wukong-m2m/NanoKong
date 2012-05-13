#include <wkpf.h>
#include <debug.h>
#include "native_wuclasses.h"

void wuclass_generic_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_generic_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_READ,
};

wkpf_wuclass_definition wuclass_generic = {
  WKPF_WUCLASS_GENERIC, // wuclass id
  wuclass_generic_update, // update function pointer
  1, // Number of properties
  wuclass_generic_properties
};

void wuclass_generic_update(wkpf_local_wuobject *wuobject) {
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Generic): Update called for generic wuclass\n");
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_GENERIC_DUMMY, 42);
}
