#include "config.h"
#include "types.h"
#include "debug.h"
#include "heap.h"
#include "delay.h"
#include "wkpf.h"
#include "wkpf_wuobjects.h"
#include "wkpf_properties.h"

#define MAX_NUMBER_OF_WUOBJECTS 10
uint8_t number_of_wuobjects;
wkpf_local_wuobject wuobjects[MAX_NUMBER_OF_WUOBJECTS];

uint8_t wkpf_create_wuobject(uint16_t wuclass_id, uint8_t port_number, heap_id_t virtual_wuclass_instance_heap_id) {
  uint8_t retval;
  wkpf_wuclass_definition *wuclass;

  if (number_of_wuobjects == MAX_NUMBER_OF_WUOBJECTS) {
    DEBUGF_WKPF("WKPF: Out of memory while creating wuobject for wuclass %x at port: FAILED\n", wuclass->wuclass_id, port_number);
    return WKPF_ERR_OUT_OF_MEMORY;
  }
  
  for (int8_t i=0; i<number_of_wuobjects; i++) {
    if (wuobjects[i].port_number == port_number) {
      DEBUGF_WKPF("WKPF: Port %x in use while creating wuobject for wuclass id %x: FAILED\n", port_number, wuclass->wuclass_id);
      return WKPF_ERR_PORT_IN_USE;
    }
  }

  retval = wkpf_get_wuclass_by_id(wuclass_id, &wuclass);
  if (retval != WKPF_OK)
    return retval;

  if (WKPF_IS_VIRTUAL_WUCLASS(wuclass) && virtual_wuclass_instance_heap_id==0)
    return WKPF_ERR_NEED_VIRTUAL_WUCLASS_INSTANCE;

  wuobjects[number_of_wuobjects].wuclass = wuclass;
  wuobjects[number_of_wuobjects].port_number = port_number;
  wuobjects[number_of_wuobjects].virtual_wuclass_instance_heap_id = virtual_wuclass_instance_heap_id;
  wuobjects[number_of_wuobjects].need_to_call_update = FALSE;
  retval = wkpf_alloc_properties_for_wuobject(&wuobjects[number_of_wuobjects]);
  if (retval != WKPF_OK)
    return retval;
  // Run update function once to initialise properties.
  wkpf_set_need_to_call_update_for_wuobject(&wuobjects[number_of_wuobjects]);
  DEBUGF_WKPF("WKPF: Created wuobject for wuclass id %x at port %x\n", wuclass->wuclass_id, port_number);

  number_of_wuobjects++;
  return WKPF_OK;
}

uint8_t wkpf_remove_wuobject(uint8_t port_number) {
  for (int8_t i=0; i<number_of_wuobjects; i++) {
    if (wuobjects[i].port_number == port_number) {
      // WuObject at index i will be removed:
      // Free allocated properties
      wkpf_free_properties_for_wuobject(&wuobjects[i]);
      // Move wuobjects at higher indexes on index down
      for (int8_t j=i+1; j<number_of_wuobjects; j++) {
        wuobjects[j-1] = wuobjects[j];
      }
      number_of_wuobjects--;
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: No wuobject at port %x found: FAILED\n", port_number);
  return WKPF_ERR_WUOBJECT_NOT_FOUND;  
}

uint8_t wkpf_get_wuobject_by_port(uint8_t port_number, wkpf_local_wuobject **wuobject) {
  for (int8_t i=0; i<number_of_wuobjects; i++) {
    if (wuobjects[i].port_number == port_number) {
      *wuobject = &wuobjects[i];
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: No wuobject at port %x found: FAILED\n", port_number);
  return WKPF_ERR_WUOBJECT_NOT_FOUND;
}

uint8_t wkpf_get_wuobject_by_index(uint8_t index, wkpf_local_wuobject **wuobject) {
  if (index >= number_of_wuobjects) {
    DEBUGF_WKPF("WKPF: No wuobject at index %x found: FAILED\n", index);
    return WKPF_ERR_WUOBJECT_NOT_FOUND;
  }
  *wuobject = &wuobjects[index];
  return WKPF_OK;  
}

uint8_t wkpf_get_wuobject_by_heap_id(heap_id_t virtual_wuclass_instance_heap_id, wkpf_local_wuobject **wuobject) {
  for (int8_t i=0; i<number_of_wuobjects; i++) {
    if (wuobjects[i].virtual_wuclass_instance_heap_id == virtual_wuclass_instance_heap_id) {
      *wuobject = &wuobjects[i];
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: no wuobject for heap_id %x found: FAILED\n", virtual_wuclass_instance_heap_id);
  return WKPF_ERR_WUOBJECT_NOT_FOUND;
}

uint8_t wkpf_get_number_of_wuobjects() {
  return number_of_wuobjects;
}

void wkpf_set_need_to_call_update_for_wuobject(wkpf_local_wuobject *wuobject) {
  // TODONR: for now just call directly for native wuclasses
  // Java update should be handled by returning from the WKPF.select() function
  if (WKPF_IS_NATIVE_WUOBJECT(wuobject))
    wuobject->wuclass->update(wuobject);
  else {
    wuobject->need_to_call_update = TRUE;
  }
}

bool wkpf_get_next_wuobject_to_update(wkpf_local_wuobject **virtual_wuobject) {
  static uint8_t last_updated_wuobject_index = 0;
  if (number_of_wuobjects == 0)
    return FALSE;
  if (last_updated_wuobject_index >= number_of_wuobjects)
    last_updated_wuobject_index = number_of_wuobjects-1; // Could happen if wuobjects were removed
  int i = last_updated_wuobject_index;
  do {
    i = (i+1) % number_of_wuobjects;
    if ((wuobjects[i].next_scheduled_update > 0 && wuobjects[i].next_scheduled_update < nvm_current_time)
        || wuobjects[i].need_to_call_update) {
      wkpf_local_wuobject *wuobject = &wuobjects[i];
      // Clear the flag if it was set
      wuobject->need_to_call_update = FALSE;
      // If update has to be called because it's scheduled, schedule the next call
      if (wuobject->next_scheduled_update > 0 && wuobject->next_scheduled_update < nvm_current_time) {
        wkpf_schedule_next_update_for_wuobject(wuobject);
      }
      // Call directly for native wuobjects, or return virtual wuobject so WKPF.select() can return it to Java
      if (WKPF_IS_NATIVE_WUOBJECT(wuobject)) {
        DEBUGF_WKPFUPDATE("WKPFUPDATE: Update native wuobject at port %x\n", wuobject->port_number);
        wuobject->wuclass->update(wuobject);
      } else { // 
        *virtual_wuobject = wuobject;
        last_updated_wuobject_index = i;
        DEBUGF_WKPFUPDATE("WKPFUPDATE: Update virtual wuobject at port %x\n", wuobject->port_number);
        return TRUE;
      }
      // Process any pending messages before continuing
      nvmcomm_poll();
    }
  } while(i != last_updated_wuobject_index);
  return FALSE;
}

bool wkpf_heap_id_in_use(heap_id_t heap_id) {
 // To prevent virtual wuclass objects from being garbage collected  
  for (int i=0; i<number_of_wuobjects; i++) {
    if (wuobjects[i].virtual_wuclass_instance_heap_id == heap_id) {
      DEBUGF_WKPF("WKPF: GC - Prevented virtual wuclass with heap_id %x from being freed.\n", heap_id);
      return TRUE;
    }
  }
  return FALSE;
}

void wkpf_schedule_next_update_for_wuobject(wkpf_local_wuobject *wuobject) {
  for (int i=0; i<wuobject->wuclass->number_of_properties; i++) {
    if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[i]) == WKPF_PROPERTY_TYPE_REFRESH_RATE) {
      wkpf_refresh_rate_t refresh_rate;
      wkpf_internal_read_property_refresh_rate(wuobject, i, &refresh_rate);
      if (refresh_rate == 0) // 0 means turned off
        wuobject->next_scheduled_update = 0;
      else
        wuobject->next_scheduled_update = nvm_current_time + refresh_rate;
      DEBUGF_WKPFUPDATE("WKPFUPDATE: Scheduled next update for object at port %x. Refresh rate:%x Current time:%08lx Next update at:%08lx\n", wuobject->port_number, refresh_rate, nvm_current_time, wuobject->next_scheduled_update);
      return;
    }
  }
}
