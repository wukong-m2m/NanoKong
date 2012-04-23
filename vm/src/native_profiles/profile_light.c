#include <debug.h>
#include <stack.h>
#include <types.h>
#include <wkpf.h>
#include <avr/native_avr.h>
#include <avr/native.h>
#include <avr/io.h>
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

  // Connect light to port L, bit 3 (Arduino digital pin 46)
  // SETOUPUT
  DDRL |= _BV(3);
  if (onOff)
    PORTL |= _BV(3);
  else
    PORTL &= ~_BV(3);
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}

#endif // ENABLE_PROFILE_LIGHT
