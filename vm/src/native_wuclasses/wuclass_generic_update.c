#include <wkpf.h>
#include <debug.h>
#include "native_wuclasses.h"

void wuclass_generic_update(wkpf_local_wuobject *wuobject) {
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Generic): Update called for generic wuclass\n");
  wkpf_internal_write_property_int16(wuobject, WKPF_PROPERTY_GENERIC_DUMMY, 42);
}
