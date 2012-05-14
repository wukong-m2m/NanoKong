#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_NUMERIC_CONTROLER

extern void wuclass_numeric_controller_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_numeric_controller_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW // WKPF_PROPERTY_NUMERIC_CONTROLLER_OUTPUT
};

wkpf_wuclass_definition wuclass_numeric_controller = {
  WKPF_WUCLASS_NUMERIC_CONTROLLER, // wuclass id
  wuclass_numeric_controller_update, // update function pointer
  1, // Number of properties
  wuclass_numeric_controller_properties
};

#endif // ENABLE_WUCLASS_NUMERIC_CONTROLER
