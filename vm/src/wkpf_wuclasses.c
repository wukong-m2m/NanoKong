#include "config.h"
#include "types.h"
#include "debug.h"
#include "wkpf.h"
#include "wkpf_wuclasses.h"

uint8_t number_of_wuclasses;
wkpf_wuclass_definition wuclasses[MAX_NUMBER_OF_WUCLASSES];

uint8_t wkpf_register_wuclass(wkpf_wuclass_definition wuclass) {
  if (number_of_wuclasses == MAX_NUMBER_OF_WUCLASSES) {
    DEBUGF_WKPF("WKPF: Out of memory while registering wuclass id %x: FAILED\n", wuclass.wuclass_id);
    return WKPF_ERR_OUT_OF_MEMORY;
  }
  for (int8_t i=0; i<number_of_wuclasses; i++) {
    if (wuclasses[i].wuclass_id == wuclass.wuclass_id) {
      DEBUGF_WKPF("WKPF: WuClass id in use while registering wuclass id %x: FAILED\n", wuclass.wuclass_id);
      return WKPF_ERR_WUCLASS_ID_IN_USE;
    }
  }
  DEBUGF_WKPF("WKPF: Registering wuclass id %x at index %x\n", wuclass.wuclass_id, number_of_wuclasses);
  wuclasses[number_of_wuclasses] = wuclass;
  number_of_wuclasses++;
  return WKPF_OK;
}

uint8_t wkpf_get_wuclass_by_id(uint16_t wuclass_id, wkpf_wuclass_definition **wuclass) {
  for (int8_t i=0; i<number_of_wuclasses; i++) {
    if (wuclasses[i].wuclass_id == wuclass_id) {
      *wuclass = &wuclasses[i];      
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: No wuclass with id %x found: FAILED\n", wuclass_id);
  return WKPF_ERR_WUCLASS_NOT_FOUND;
}

uint8_t wkpf_get_wuclass_by_index(uint8_t index, wkpf_wuclass_definition **wuclass) {
  if (index >= number_of_wuclasses) {
    DEBUGF_WKPF("WKPF: No wuclass at index %x found: FAILED\n", index);
    return WKPF_ERR_WUCLASS_NOT_FOUND;
  }
  *wuclass = &wuclasses[index];
  return WKPF_OK;
}

uint8_t wkpf_get_number_of_wuclasses() {
  return number_of_wuclasses;
}
