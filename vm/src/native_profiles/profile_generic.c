#include <wkpf.h>
#include <debug.h>
#include "native_profiles.h"

void profile_generic_update(wkpf_local_endpoint *endpoint);

uint8_t profile_generic_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_READ,
};

wkpf_profile_definition profile_generic = {
  WKPF_PROFILE_ID_GENERIC, // profile id
  profile_generic_update, // update function pointer
  NULL, // Java object
  1, // Number of properties
  profile_generic_properties
};

void profile_generic_update(wkpf_local_endpoint *endpoint) {
  DEBUGF_WKPF("WKPF: Update called for generic profile\n");
  wkpf_internal_write_property_int16(endpoint, WKPF_PROPERTY_ID_GENERIC_DUMMY, 42);
}
