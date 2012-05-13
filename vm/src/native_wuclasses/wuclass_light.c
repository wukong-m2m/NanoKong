#include <debug.h>
#include <stack.h>
#include <types.h>
#include <wkpf.h>
#include <avr/native_avr.h>
#include <avr/native.h>
#include <avr/io.h>
#include "native_wuclasses.h"
#include "wuclass_light.h"

#ifdef ENABLE_WUCLASS_LIGHT

void wuclass_light_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_light_properties[] = {
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_RW // WKPF_PROPERTY_LIGHT_ONOFF
};

wkpf_wuclass_definition wuclass_light = {
  WKPF_WUCLASS_LIGHT, // wuclass id
  wuclass_light_update, // update function pointer
  1, // Number of properties
  wuclass_light_properties
};

void wuclass_light_update(wkpf_local_wuobject *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ONOFF, &onOff);

  // Connect light to port L, bit 3 (Arduino digital pin 46)
  // SETOUPUT
  DDRL |= _BV(3);
  if (onOff)
    PORTL |= _BV(3);
  else
    PORTL &= ~_BV(3);
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}

#endif // ENABLE_WUCLASS_LIGHT
