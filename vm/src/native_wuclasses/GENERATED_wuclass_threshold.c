#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_THRESHOLD

extern void wuclass_threshold_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_threshold_properties[] = {
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE, // WKPF_PROPERTY_THRESHOLD_OPERATOR
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE, // WKPF_PROPERTY_THRESHOLD_THRESHOLD
  WKPF_PROPERTY_TYPE_SHORT+WKPF_PROPERTY_ACCESS_READWRITE, // WKPF_PROPERTY_THRESHOLD_VALUE
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READONLY // WKPF_PROPERTY_THRESHOLD_OUTPUT
};

wkpf_wuclass_definition wuclass_threshold = {
  WKPF_WUCLASS_THRESHOLD, // wuclass id
  wuclass_threshold_update, // update function pointer
  4, // Number of properties
  wuclass_threshold_properties
};

#endif // ENABLE_WUCLASS_THRESHOLD
