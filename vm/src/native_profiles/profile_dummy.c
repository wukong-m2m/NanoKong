#include <wkpf.h>
#include "native_profiles.h"

#ifdef ENABLE_PROFILE_DUMMY

void profile_dummy_update(wkpf_local_endpoint *endpoint);

uint8_t profile_dummy_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_READ,
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW,
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW
};

wkpf_profile_definition profile_dummy = {
  WKPF_PROFILE_ID_DUMMY, // profile id
  profile_dummy_update, // update function pointer
  NULL, // Java object
  3, // Number of properties
  profile_dummy_properties
};

void profile_dummy_update(wkpf_local_endpoint *endpoint) {
  return;
}

#endif // ENABLE_PROFILE_DUMMY
