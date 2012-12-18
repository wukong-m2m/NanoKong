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

  // Connect light to port B, bit 4. This maps to pin 3 of JP18 on the WuNode (pin 1 is behind the power connector)
  // SETOUPUT
  DDRB |= _BV(4);
  DDRB |= _BV(7);
  if (onOff)
    PORTB |= _BV(4);
  else
    PORTB &= ~_BV(4);
  if (onOff)
    PORTB |= _BV(7);
  else
    PORTB &= ~_BV(7);
  DEBUGF_WKPFUPDATE("WKPFUPDATE(Light): Setting light to: %x\n", onOff);
}

#endif // ENABLE_WUCLASS_LIGHT_ACTUATOR
