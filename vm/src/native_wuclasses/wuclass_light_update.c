#include <debug.h>
#include <stack.h>
#include <types.h>
#include <wkpf.h>
#include <avr/native_avr.h>
#include <avr/native.h>
#include <avr/io.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_LIGHT

void wuclass_light_update(wkpf_local_wuobject *wuobject) {
  bool onOff;
  wkpf_internal_read_property_boolean(wuobject, WKPF_PROPERTY_LIGHT_ON_OFF, &onOff);

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
