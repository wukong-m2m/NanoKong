#include <debug.h>
#include <wkpf.h>
#include "native_wuclasses.h"

#ifdef ENABLE_WUCLASS_THRESHOLD

void wuclass_threshold_update(wkpf_local_wuobject *wuobject) {
  int16_t operator;
  int16_t threshold;
  int16_t value;

  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_OPERATOR, &operator);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_THRESHOLD, &threshold);
  wkpf_internal_read_property_int16(wuobject, WKPF_PROPERTY_THRESHOLD_VALUE, &value);

	if (((operator == WKPF_ENUM_THRESHOLD_OPERATOR_GT || operator == WKPF_ENUM_THRESHOLD_OPERATOR_GTE) && value > threshold)
	 || ((operator == WKPF_ENUM_THRESHOLD_OPERATOR_LT || operator == WKPF_ENUM_THRESHOLD_OPERATOR_LTE) && value < threshold)
	 || ((operator == WKPF_ENUM_THRESHOLD_OPERATOR_GTE || operator == WKPF_ENUM_THRESHOLD_OPERATOR_LTE) && value == threshold)) {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, TRUE);
    DEBUGF_WKPFUPDATE("WKPFUPDATE(Threshold): Native threshold: operator %x threshold %x value %x -> TRUE\n", operator, threshold, value);
  }
	else {
    wkpf_internal_write_property_boolean(wuobject, WKPF_PROPERTY_THRESHOLD_OUTPUT, FALSE);
    DEBUGF_WKPFUPDATE("WKPFUPDATE(Threshold): Native threshold: operator %x threshold %x value %x -> FALSE\n", operator, threshold, value);
  }
}

#endif // ENABLE_WUCLASS_THRESHOLD
