#include <wkpf_profiles.h>
#include <wkpf_endpoints.h>

#include "native_profiles.h"

uint8_t register_profile_and_create_endpoint(wpkf_profile *profile, uint8_t port_number) {
  retval = wkpf_register_profile(*profile);
  if (retval != WKPF_OK)
    return retval;
  retval = wkpf_create_endpoint(profile->profile_id, port_number);
  if (retval != WKPF_OK)
    return retval;
}

uint8_t native_profiles_init() {
  uint8_t retval;

  retval = register_profile_and_create_endpoint(&profile_generic, 0x0); // Always create endpoint for generic profile at port 0
  if (retval != WKPF_OK)
    return retval;
  
#ifdef ENABLE_PROFILE_DUMMY
  retval = register_profile_and_create_endpoint(&profile_dummy, 0x1);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_PROFILE_DUMMY

  return WKPF_OK;
}
