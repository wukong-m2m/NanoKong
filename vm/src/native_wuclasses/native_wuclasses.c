#include <debug.h>
#include <wkpf.h>

#include "native_wuclasses.h"
#include "GENERATED_wuclass_generic.h"
#include "GENERATED_wuclass_threshold.h"
#include "GENERATED_wuclass_numeric_controller.h"
#include "GENERATED_wuclass_light.h"
#include "GENERATED_wuclass_light_sensor.h"

uint8_t register_wuclass_and_create_wuobject(wkpf_wuclass_definition *wuclass, uint8_t port_number) {
  uint8_t retval = wkpf_register_wuclass(*wuclass);
  if (retval != WKPF_OK)
    return retval;
  retval = wkpf_create_wuobject(wuclass->wuclass_id, port_number, 0);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t native_wuclasses_init() {
  uint8_t retval;

  retval = register_wuclass_and_create_wuobject(&wuclass_generic, 0x0); // Always create wuobject for generic wuclass at port 0
  if (retval != WKPF_OK)
    return retval;
  
#ifdef ENABLE_WUCLASS_THRESHOLD
  retval = wkpf_register_wuclass(wuclass_threshold);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_WUCLASS_THRESHOLD

#ifdef ENABLE_WUCLASS_NUMERIC_CONTROLER
  retval = wkpf_register_wuclass(wuclass_numeric_controller);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_WUCLASS_NUMERIC_CONTROLER

#ifdef ENABLE_WUCLASS_LIGHT
  retval = wkpf_register_wuclass(wuclass_light);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_WUCLASS_LIGHT

#ifdef ENABLE_WUCLASS_LIGHT_SENSOR
  retval = wkpf_register_wuclass(wuclass_light_sensor);
  if (retval != WKPF_OK)
    return retval;
#endif // ENABLE_WUCLASS_LIGHT_SENSOR

  // TODONR TMP hard code everything on this node
  retval = wkpf_create_wuobject(wuclass_numeric_controller.wuclass_id, 1, 0); // Input
  if (retval != WKPF_OK)
    DEBUGF_WKPF("WKPF: error creating input wuobject retval = %x\n", retval);
  retval = wkpf_create_wuobject(wuclass_light_sensor.wuclass_id, 2, 0); // Light sensor
  if (retval != WKPF_OK)
    DEBUGF_WKPF("WKPF: error creating light sensor wuobject retval = %x\n", retval);
  retval = wkpf_create_wuobject(wuclass_light.wuclass_id, 4, 0); // Light
  if (retval != WKPF_OK)
    DEBUGF_WKPF("WKPF: error creating light wuobject retval = %x\n", retval);

  return WKPF_OK;
}
