#include <debug.h>
#include <stack.h>
#include <types.h>
#include <wkpf.h>
#include <avr/native_avr.h>
#include <avr/native.h>
#include <avr/io.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_LIGHT_ACTUATOR

void wuclass_light_actuator_update(wkpf_local_wuobject *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ACTUATOR_ON_OFF, &onOff);

  // Connect light to port L, bit 4 (Arduino digital pin 45)
  // SETOUPUT
  DDRL |= _BV(4);
  if (onOff)
    PORTL |= _BV(4);
  else
    PORTL &= ~_BV(4);
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}

#endif // ENABLE_WUCLASS_LIGHT_ACTUATOR
