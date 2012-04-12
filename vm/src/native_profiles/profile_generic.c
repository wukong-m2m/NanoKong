#include <wkpf.h>
#include "native_profiles.h"

void profile_generic_update(wkpf_local_endpoint *endpoint);

uint8_t profile_generic_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_READ,
};

wkpf_profile_definition profile_generic = {
  0xFF42, // profile id
  profile_generic_update, // update function pointer
  NULL, // Java object
  3, // Number of properties
  profile_generic_properties
};

void profile_generic_update(wkpf_local_endpoint *endpoint) {

}

