#ifndef NATIVE_WUCLASSESH
#define NATIVE_WUCLASSESH

#include <wkpf_wuclasses.h>
#include "wuclass_generic.h"
#include "wuclass_threshold.h"
#include "wuclass_numeric_controller.h"
#include "wuclass_light.h"
#include "wuclass_light_sensor.h"

#define ENABLE_WUCLASS_THRESHOLD
#define ENABLE_WUCLASS_NUMERIC_CONTROLER
#define ENABLE_WUCLASS_LIGHT
#define ENABLE_WUCLASS_LIGHT_SENSOR

uint8_t native_wuclasses_init();

#endif // NATIVE_WUCLASSESH