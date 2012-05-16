#include "config.h"
#include "types.h"
#include "debug.h"
#include "wkpf.h"
#include "wkpf_properties.h"

#define DIRTY_STATE_CLEAN     0
#define DIRTY_STATE_DIRTY     1
#define DIRTY_STATE_FAILED    2

typedef struct property_entry_struct {
  uint8_t wuobject_port_number;
  uint8_t property_number;
  int16_t value;
  uint8_t dirty_state;
} property_entry;

#define MAX_NUMBER_OF_PROPERTIES 100
uint8_t number_of_properties;
property_entry properties[MAX_NUMBER_OF_PROPERTIES];

uint8_t wkpf_read_property(wkpf_local_wuobject *wuobject, uint8_t property_number, int16_t *value) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].wuobject_port_number == wuobject->port_number && properties[i].property_number == property_number) {
      DEBUGF_WKPF("WKPF: read_property: (index %x port %x, property %x): %x\n", i, wuobject->port_number, property_number, properties[i].value);
      *value = properties[i].value;
      return WKPF_OK;
    }
  }
  return WKPF_ERR_SHOULDNT_HAPPEN;
}
uint8_t wkpf_write_property(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, int16_t value) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].wuobject_port_number == wuobject->port_number && properties[i].property_number == property_number) {
      DEBUGF_WKPF("WKPF: write_property: (index %x port %x, property %x): %x->%x\n", i, wuobject->port_number, property_number, properties[i].value, value);
      if (properties[i].value != value) {
        properties[i].value = value;
        properties[i].dirty_state = DIRTY_STATE_DIRTY;
        if (external_access) // Only call update() when someone else writes to the property, not for internal writes (==writes that are already coming from update())
          wkpf_set_need_to_call_update_for_wuobject(wuobject);
      }
      return WKPF_OK;
    }
  }
  return WKPF_ERR_SHOULDNT_HAPPEN;
}

uint8_t wkpf_verify_property(wkpf_local_wuobject *wuobject, uint8_t property_number, uint8_t access, bool external_access, uint8_t type) {
  if (wuobject->wuclass->number_of_properties <= property_number)
    return WKPF_ERR_PROPERTY_NOT_FOUND;
  uint8_t property = wuobject->wuclass->properties[property_number];
  if (external_access) {
    if (access == WKPF_PROPERTY_ACCESS_READ && WKPF_IS_WRITEONLY_PROPERTY(property))
      return WKPF_ERR_WRITE_ONLY;
    if (access == WKPF_PROPERTY_ACCESS_WRITE && WKPF_IS_READONLY_PROPERTY(property))
      return WKPF_ERR_READ_ONLY;
  }
  if (type != WKPF_GET_PROPERTY_DATATYPE(property))
    return WKPF_ERR_WRONG_DATATYPE;
  return WKPF_OK;
}

uint8_t wkpf_read_property_int16(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, int16_t *value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_READ, external_access, WKPF_PROPERTY_TYPE_SHORT);
  if (retval == WKPF_OK)
    return wkpf_read_property(wuobject, property_number, value);
  else
    return retval;
}
uint8_t wkpf_write_property_int16(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, int16_t value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITE, external_access, WKPF_PROPERTY_TYPE_SHORT);
  if (retval == WKPF_OK)
    return wkpf_write_property(wuobject, property_number, external_access, value);
  else
    return retval;
}

uint8_t wkpf_read_property_boolean(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, bool *value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_READ, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);

  if (retval == WKPF_OK) {
    int16_t value_16bit;
    retval = wkpf_read_property(wuobject, property_number, &value_16bit);
    if (retval == WKPF_OK) {
      *value = (int8_t)value_16bit;
    }
    return retval;
  }
  else
    return retval;
}
uint8_t wkpf_write_property_boolean(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, bool value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITE, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);
  if (retval == WKPF_OK)
    return wkpf_write_property(wuobject, property_number, external_access, value);
  else
    return retval;
}

uint8_t wkpf_read_property_refresh_rate(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t *value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_READ, external_access, WKPF_PROPERTY_TYPE_REFRESH_RATE);
  if (retval == WKPF_OK)
    return wkpf_read_property(wuobject, property_number, value);
  else
    return retval;
}
uint8_t wkpf_write_property_refresh_rate(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITE, external_access, WKPF_PROPERTY_TYPE_REFRESH_RATE);
  if (retval == WKPF_OK) {
    retval = wkpf_write_property(wuobject, property_number, external_access, value);
    wkpf_schedule_next_update_for_wuobject(wuobject);
    return retval;
  } else
    return retval;
}

uint8_t wkpf_alloc_properties_for_wuobject(wkpf_local_wuobject *wuobject) {
  for (int i=0; i<number_of_properties; i++)
    if (properties[i].wuobject_port_number == wuobject->port_number)
      return WKPF_ERR_WUOBJECT_ALREADY_ALLOCATED;
  if (number_of_properties+wuobject->wuclass->number_of_properties > MAX_NUMBER_OF_PROPERTIES)
    return WKPF_ERR_OUT_OF_MEMORY;
  for (int i=0; i<wuobject->wuclass->number_of_properties; i++) {
    properties[number_of_properties+i].wuobject_port_number = wuobject->port_number;
    properties[number_of_properties+i].property_number = i;
    properties[number_of_properties+i].value = 0;
    properties[number_of_properties+i].dirty_state = DIRTY_STATE_DIRTY;
  }
  number_of_properties += wuobject->wuclass->number_of_properties;
  DEBUGF_WKPF("WKPF: Allocated %x properties for wuobject at port %x. number of properties is now %x\n", wuobject->wuclass->number_of_properties, wuobject->port_number, number_of_properties);
  return WKPF_OK;
}

uint8_t wkpf_free_properties_for_wuobject(wkpf_local_wuobject *wuobject) {
  int16_t first=-1, last=-1;
  // Find the sequence of properties corresponding to this wuobject
  for (int i=0; i<number_of_properties; i++) {
    if (first==-1 && properties[i].wuobject_port_number==wuobject->port_number)
      first = i;
    if (first!=-1 && properties[i].wuobject_port_number!=wuobject->port_number)
      last = i;
  }
  if (last != -1) // If there are any, and there are other properties following after, shift them down to compact the property list
    while (last<number_of_properties)
      properties[first++] = properties[last++];
  number_of_properties -= wuobject->wuclass->number_of_properties;
  return WKPF_OK;
}

bool wkpf_any_property_dirty() {
  // This will be called from the native select() function. Mark updates that have failed as dirty again to give them another chance.
  bool dirty = FALSE;
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].dirty_state == DIRTY_STATE_DIRTY)
      dirty = TRUE;
    if (properties[i].dirty_state == DIRTY_STATE_FAILED)
      properties[i].dirty_state = DIRTY_STATE_DIRTY;
  }
  return dirty;
}

static uint8_t last_returned_dirty_property_index = 0;
bool wkpf_get_next_dirty_property(uint8_t *port_number, uint8_t *property_number, int16_t *value) {
  if (last_returned_dirty_property_index >= number_of_properties)
    last_returned_dirty_property_index = number_of_properties-1; // Could happen if wuobjects were removed
  int i = last_returned_dirty_property_index;

  do {
    i = (i+1) % number_of_properties;
    if (properties[i].dirty_state == DIRTY_STATE_DIRTY) {
      last_returned_dirty_property_index = i;
      *port_number = properties[i].wuobject_port_number;
      *property_number = properties[i].property_number;
      *value = properties[i].value;
      properties[i].dirty_state = DIRTY_STATE_CLEAN;
//      DEBUGF_WKPF("WKPF: wkpf_get_next_dirty_property DIRTY[%x]: port %x property %x retval %x\n", i, properties[i].wuobject_port_number, properties[i].property_number, ((uint16_t)properties[i].wuobject_port_number)<<8 | properties[i].property_number);
      return TRUE;
    }
//    DEBUGF_WKPF("NOT DIRTY[%x]: port %x property %x retval %x\n", i, properties[i].wuobject_port_number, properties[i].property_number, ((uint16_t)properties[i].wuobject_port_number)<<8 | properties[i].property_number);
  } while(i != last_returned_dirty_property_index);
  return FALSE;
}

void wkpf_propagating_dirty_property_failed(uint8_t port_number, uint8_t property_number) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].wuobject_port_number == port_number
        && properties[i].property_number == property_number)
      // Mark them as failed and not as dirty again to prevent endless retries. Now we wait until the next call to update() which will be some time later as long as we have the main loop in Java.
      properties[i].dirty_state = DIRTY_STATE_FAILED;
  }
}

