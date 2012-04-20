#include <debug.h>
#include <wkpf.h>

#include "native_profiles.h"

uint8_t register_profile_and_create_endpoint(wkpf_profile_definition *profile, uint8_t port_number) {
  uint8_t retval = wkpf_register_profile(*profile);
  if (retval != WKPF_OK)
    return retval;
  retval = wkpf_create_endpoint(profile->profile_id, port_number, 0);
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
  retval = wkpf_register_profile(profile_threshold);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_PROFILE_THRESHOLD

#ifdef ENABLE_PROFILE_NUMERIC_CONTROLER
  retval = wkpf_register_profile(profile_numeric_controller);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_PROFILE_NUMERIC_CONTROLER

#ifdef ENABLE_PROFILE_LIGHT
  retval = wkpf_register_profile(profile_light);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_PROFILE_LIGHT

#ifdef ENABLE_PROFILE_LIGHT_SENSOR
  retval = wkpf_register_profile(profile_light_sensor);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_PROFILE_LIGHT_SENSOR

  // TODONR TMP hard code everything on this node
  retval = wkpf_create_endpoint(profile_numeric_controller.profile_id, 1, 0); // Thermostat
  if (retval != WKPF_OK)
    DEBUGF_WKPF("retval = %x\n", retval);
  retval = wkpf_create_endpoint(profile_light_sensor.profile_id, 2, 0); // Light sensor
  if (retval != WKPF_OK)
    DEBUGF_WKPF("retval = %x\n", retval);
  retval = wkpf_create_endpoint(profile_light.profile_id, 4, 0); // Light
  if (retval != WKPF_OK)
    DEBUGF_WKPF("retval = %x\n", retval);

  return WKPF_OK;
}
