#include <debug.h>
#include <stack.h>
#include <types.h>
#include <wkpf.h>
#include <avr/native_avr.h>
#include <avr/native.h>
#include "native_profiles.h"
#include "profile_light.h"

#ifdef ENABLE_PROFILE_LIGHT

void profile_light_update(wkpf_local_endpoint *endpoint);

uint8_t profile_light_properties[] = {
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW // WKPF_PROPERTY_LIGHT_ONOFF
};

wkpf_profile_definition profile_light = {
  WKPF_PROFILE_LIGHT, // profile id
  profile_light_update, // update function pointer
  1, // Number of properties
  profile_light_properties
};

void profile_light_update(wkpf_local_endpoint *endpoint) {
  bool onOff;
  wkpf_internal_read_property_boolean(endpoint, WKPF_PROPERTY_LIGHT_ONOFF, &onOff);
  
  // Abuse native AVR code for now.
  // Connect light to port B, bit 0
  stack_push(1); // Port B
  stack_push(0); // Bit 0
  native_avr_port_invoke(NATIVE_METHOD_SETOUTPUT);
  stack_push(1); // Port B
  stack_push(0); // Bit 0
  if (onOff)
    native_avr_port_invoke(NATIVE_METHOD_SETBIT);
  else
    native_avr_port_invoke(NATIVE_METHOD_CLRBIT);
}

#endif // ENABLE_PROFILE_LIGHT
