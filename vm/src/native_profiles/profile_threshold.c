#include <debug.h>
#include <wkpf.h>
#include "native_profiles.h"
#include "profile_threshold.h"

#ifdef ENABLE_PROFILE_THRESHOLD

void profile_threshold_update(wkpf_local_endpoint *endpoint);

uint8_t profile_threshold_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, // WKPF_PROPERTY_THRESHOLD_OPERATOR
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, // WKPF_PROPERTY_THRESHOLD_THRESHOLD
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, // WKPF_PROPERTY_THRESHOLD_VALUE
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READ // WKPF_PROPERTY_THRESHOLD_OUTPUT
};

wkpf_profile_definition profile_threshold = {
  WKPF_PROFILE_THRESHOLD, // profile id
  profile_threshold_update, // update function pointer
  4, // Number of properties
  profile_threshold_properties
};

void profile_threshold_update(wkpf_local_endpoint *endpoint) {
  int16_t operator;
  int16_t threshold;
  int16_t value;

  wkpf_internal_read_property_int16(endpoint, WKPF_PROPERTY_THRESHOLD_OPERATOR, &operator);
  wkpf_internal_read_property_int16(endpoint, WKPF_PROPERTY_THRESHOLD_THRESHOLD, &threshold);
  wkpf_internal_read_property_int16(endpoint, WKPF_PROPERTY_THRESHOLD_VALUE, &value);

	if ((operator == THRESHOLD_PROFILE_OPERATOR_GT && value > threshold)
	 || (operator == THRESHOLD_PROFILE_OPERATOR_LT && value < threshold)) {
    wkpf_internal_write_property_boolean(endpoint, WKPF_PROPERTY_THRESHOLD_OUTPUT, TRUE);
  }
	else {
    wkpf_internal_write_property_boolean(endpoint, WKPF_PROPERTY_THRESHOLD_OUTPUT, FALSE);
  }
}

#endif // ENABLE_PROFILE_THRESHOLD
