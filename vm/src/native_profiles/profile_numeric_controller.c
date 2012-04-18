#include <debug.h>
#include <wkpf.h>
#include "native_profiles.h"
#include "profile_numeric_controller.h"

#ifdef ENABLE_PROFILE_NUMERIC_CONTROLER

void profile_numeric_controller_update(wkpf_local_endpoint *endpoint);

uint8_t profile_numeric_controller_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW // WKPF_PROPERTY_NUMERIC_CONTROLLER_OUTPUT
};

wkpf_profile_definition profile_numeric_controller = {
  WKPF_PROFILE_NUMERIC_CONTROLLER, // profile id
  profile_numeric_controller_update, // update function pointer
  1, // Number of properties
  profile_numeric_controller_properties
};

void profile_numeric_controller_update(wkpf_local_endpoint *endpoint) {
}

#endif // ENABLE_PROFILE_NUMERIC_CONTROLER
