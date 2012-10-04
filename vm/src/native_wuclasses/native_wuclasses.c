#include <debug.h>
#include <wkpf.h>
#include <nvmcomm.h>

#include "native_wuclasses.h"
#include "GENERATEDwuclass_generic.h"
#include "GENERATEDwuclass_threshold.h"
#include "GENERATEDwuclass_numeric_controller.h"
#include "GENERATEDwuclass_light_actuator.h"
#include "GENERATEDwuclass_light_sensor.h"

uint8_t wkpf_register_wuclass_and_create_wuobject(wkpf_wuclass_definition wuclass, uint8_t port_number) {
  uint8_t retval = wkpf_register_wuclass(wuclass);
  if (retval != WKPF_OK)
    return retval;
  retval = wkpf_create_wuobject(wuclass.wuclass_id, port_number, 0);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t wkpf_init_node1() {
  uint8_t retval;
  
  // Light sensor
  retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_sensor, 2);
  if (retval != WKPF_OK)
    return retval;

  // Numeric controller
  retval = wkpf_register_wuclass_and_create_wuobject(wuclass_numeric_controller, 1);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t wkpf_init_node3() {
  uint8_t retval;
  
  // Light
  retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_actuator, 4);
  if (retval != WKPF_OK)
    return retval;

  // Threshold
  retval = wkpf_register_wuclass(wuclass_threshold);
  if (retval != WKPF_OK)
    return retval;
  return WKPF_OK;
}

uint8_t wkpf_native_wuclasses_init() {
  uint8_t retval;

  /*
  retval = wkpf_register_wuclass_and_create_wuobject(wuclass_generic, 0); // Always create wuobject for generic wuclass at port 0
  if (retval != WKPF_OK)
    return retval;
  */
  
  DEBUGF_WKPF("Running wkpf native init for node id: %x\n", nvmcomm_get_node_id());

  // Light actuator
  retval = wkpf_register_wuclass(wuclass_light_actuator);
  /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_actuator, 4);*/
  if (retval != WKPF_OK)
    return retval;

  // Light sensor
  retval = wkpf_register_wuclass(wuclass_light_sensor);
  /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_sensor, 2);*/
  if (retval != WKPF_OK)
    return retval;

  // Numeric controller
  retval = wkpf_register_wuclass(wuclass_numeric_controller);
  /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_numeric_controller, 1);*/
  if (retval != WKPF_OK)
    return retval;

  return WKPF_OK;

  /*
  if (nvmcomm_get_node_id() == 1)
    return wkpf_init_node1();
  else if (nvmcomm_get_node_id() == 3)
    return wkpf_init_node3();
  else
    return WKPF_OK;
  */
}
