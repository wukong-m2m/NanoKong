#include <debug.h>
#include <wkpf.h>
#include "native_wuclasses.h"
#include "wuclass_threshold.h"

#ifdef ENABLE_WUCLASS_THRESHOLD

void wuclass_threshold_update(wkpf_local_wuobject *wuobject);

uint8_t wuclass_threshold_properties[] = {
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, // WKPF_PROPERTY_THRESHOLD_OPERATOR
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, // WKPF_PROPERTY_THRESHOLD_THRESHOLD
  WKPF_PROPERTY_TYPE_INT16+WKPF_PROPERTY_ACCESS_RW, // WKPF_PROPERTY_THRESHOLD_VALUE
  WKPF_PROPERTY_TYPE_BOOLEAN+WKPF_PROPERTY_ACCESS_READ // WKPF_PROPERTY_THRESHOLD_OUTPUT
};

wkpf_wuclass_definition wuclass_threshold = {
  WKPF_WUCLASS_THRESHOLD, // wuclass id
  wuclass_threshold_update, // update function pointer
  4, // Number of properties
  wuclass_threshold_properties
};

void wuclass_threshold_update(wkpf_local_wuobject *wuobject) {
  int16_t operator;
  int16_t threshold;
  int16_t value;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_OPERATOR, &operator);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_THRESHOLD, &threshold);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_VALUE, &value);

	if (((operator == OPERATOR_GT || operator == WKPF_ENUM_THRESHOLD_OPERATOR_GTE) && value > threshold)
	 || ((operator == OPERATOR_LT || operator == WKPF_ENUM_THRESHOLD_OPERATOR_LTE) && value < threshold)
	 || ((operator == OPERATOR_GTE || operator == WKPF_ENUM_THRESHOLD_OPERATOR_LTE) && value == threshold)) {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, TRUE);
    DEBUGF_WKPFUPDATE("WKPFUPDATE(Threshold): Native threshold: operator %x threshold %x value %x -> TRUE\n", operator, threshold, value);
  }
	else {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, FALSE);
    DEBUGF_WKPFUPDATE("WKPFUPDATE(Threshold): Native threshold: operator %x threshold %x value %x -> FALSE\n", operator, threshold, value);
  }
}

#endif // ENABLE_WUCLASS_THRESHOLD
