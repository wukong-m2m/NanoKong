#include <debug.h>
#include <wkpf.h>

#include "native_profiles.h"

uint8_t register_profile_and_create_endpoint(wkpf_profile_definition *profile, uint8_t port_number) {
  uint8_t retval = wkpf_register_profile(*profile);
  if (retval != WKPF_OK)
    return retval;
  retval = wkpf_create_endpoint(profile->profile_id, port_number, NULL);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t native_profiles_init() {
  uint8_t retval;

  retval = register_profile_and_create_endpoint(&profile_generic, 0x0); // Always create endpoint for generic profile at port 0
  if (retval != WKPF_OK)
    return retval;
  
#ifdef ENABLE_PROFILE_THRESHOLD
  retval = register_profile_and_create_endpoint(&profile_threshold, 0x1);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_PROFILE_THRESHOLD

  return WKPF_OK;
}
