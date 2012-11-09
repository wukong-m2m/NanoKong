#include <debug.h>
#include <wkpf.h>
#include <nvmcomm.h>
#include "wkpf_config.h"
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

uint8_t wkpf_native_wuclasses_init() {
  uint8_t retval;

  retval = wkpf_register_wuclass_and_create_wuobject(wuclass_generic, 0); // Always create wuobject for generic wuclass at port 0
  if (retval != WKPF_OK)
    return retval;

  DEBUGF_WKPF("Running wkpf native init for node id: %x\n", nvmcomm_get_node_id());

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_LIGHT_SENSOR)) {
    retval = wkpf_register_wuclass(wuclass_light_sensor);
    /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_sensor, 1);*/
    if (retval != WKPF_OK)
      return retval;
  }

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_LIGHT_ACTUATOR)) {
    retval = wkpf_register_wuclass(wuclass_light_actuator);
    /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_light_actuator, 2);*/
    if (retval != WKPF_OK)
      return retval;
  }

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_NUMERIC_CONTROLLER)) {
    retval = wkpf_register_wuclass(wuclass_numeric_controller);
    /*retval = wkpf_register_wuclass_and_create_wuobject(wuclass_numeric_controller, 3);*/
    if (retval != WKPF_OK)
      return retval;
  }

  if (wkpf_config_get_feature_enabled(WPKF_FEATURE_NATIVE_THRESHOLD)) {
    retval = wkpf_register_wuclass(wuclass_threshold);
    if (retval != WKPF_OK)
      return retval;
  }

  return WKPF_OK;
}
