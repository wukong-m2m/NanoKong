#ifndef NATIVE_PROFILESH
#define NATIVE_PROFILESH

#include <wkpf_profiles.h>
#include "profile_generic.h"
#include "profile_threshold.h"
#include "profile_numeric_controller.h"
#include "profile_light.h"
#include "profile_light_sensor.h"

#define ENABLE_PROFILE_THRESHOLD
#define ENABLE_PROFILE_NUMERIC_CONTROLER
#define ENABLE_PROFILE_LIGHT
#define ENABLE_PROFILE_LIGHT_SENSOR

uint8_t native_profiles_init();

#endif // NATIVE_PROFILESH