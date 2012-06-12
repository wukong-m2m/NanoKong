#include "config.h"
#include "types.h"
#include "debug.h"
#include "delay.h"
#include "wkpf.h"
#include "wkpf_properties.h"

typedef struct property_entry_struct {
  uint8_t wuobject_port_number;
  uint8_t property_number;
  int16_t value;
  uint8_t property_status;
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
        // Propagate this property:
        properties[i].property_status |= PROPERTY_STATUS_NEEDS_PUSH;
        // And remove any flags that indicate we were waiting for an initial value:
        properties[i].property_status &= ~PROPERTY_STATUS_NEEDS_PULL;
        properties[i].property_status &= ~PROPERTY_STATUS_NEEDS_PULL_WAITING;
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
    if (access == WKPF_PROPERTY_ACCESS_READONLY && WKPF_IS_WRITEONLY_PROPERTY(property))
      return WKPF_ERR_WRITE_ONLY;
    if (access == WKPF_PROPERTY_ACCESS_WRITEONLY && WKPF_IS_READONLY_PROPERTY(property))
      return WKPF_ERR_READ_ONLY;
  }
  if (type != WKPF_GET_PROPERTY_DATATYPE(property))
    return WKPF_ERR_WRONG_DATATYPE;
  return WKPF_OK;
}

uint8_t wkpf_read_property_int16(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, int16_t *value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_READONLY, external_access, WKPF_PROPERTY_TYPE_SHORT);
  if (retval == WKPF_OK)
    return wkpf_read_property(wuobject, property_number, value);
  else
    return retval;
}
uint8_t wkpf_write_property_int16(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, int16_t value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITEONLY, external_access, WKPF_PROPERTY_TYPE_SHORT);
  if (retval == WKPF_OK)
    return wkpf_write_property(wuobject, property_number, external_access, value);
  else
    return retval;
}

uint8_t wkpf_read_property_boolean(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, bool *value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_READONLY, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);

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
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITEONLY, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);
  if (retval == WKPF_OK)
    return wkpf_write_property(wuobject, property_number, external_access, value);
  else {
    DEBUGF_WKPF("--------------------------- %x\n", retval);
    return retval;
  }
}

uint8_t wkpf_read_property_refresh_rate(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t *value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_READONLY, external_access, WKPF_PROPERTY_TYPE_REFRESH_RATE);
  if (retval == WKPF_OK)
    return wkpf_read_property(wuobject, property_number, value);
  else
    return retval;
}
uint8_t wkpf_write_property_refresh_rate(wkpf_local_wuobject *wuobject, uint8_t property_number, bool external_access, wkpf_refresh_rate_t value) {
  uint8_t retval = wkpf_verify_property(wuobject, property_number, WKPF_PROPERTY_ACCESS_WRITEONLY, external_access, WKPF_PROPERTY_TYPE_REFRESH_RATE);
  if (retval == WKPF_OK) {
    retval = wkpf_write_property(wuobject, property_number, external_access, value);
    wkpf_schedule_next_update_for_wuobject(wuobject);
    return retval;
  } else
    return retval;
}

uint8_t wkpf_get_property_status(wkpf_local_wuobject *wuobject, uint8_t property_number, uint8_t *status) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].wuobject_port_number == wuobject->port_number && properties[i].property_number == property_number) {
      DEBUGF_WKPF("WKPF: wkpf_get_property_status: (index %x port %x, property %x): %x\n", i, wuobject->port_number, property_number, properties[i].property_status);
      *status = properties[i].property_status;
      return WKPF_OK;
    }
  }
  return WKPF_ERR_PROPERTY_NOT_FOUND;
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
    // Set the needs push bit
    properties[number_of_properties+i].property_status = 0;
    if (wkpf_does_property_need_initialisation_pull(wuobject->port_number, i)) {
      properties[number_of_properties+i].property_status |= PROPERTY_STATUS_NEEDS_PULL;
      DEBUGF_WKPF("WKPF: Setting needs pull bit for property %x at port %x\n", i, wuobject->port_number);
    }
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

uint8_t wkpf_property_needs_initialisation_push(wkpf_local_wuobject *wuobject, uint8_t property_number) {
  if (wuobject->wuclass->number_of_properties <= property_number)
    return WKPF_ERR_PROPERTY_NOT_FOUND;
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].wuobject_port_number == wuobject->port_number && properties[i].property_number == property_number) {
      if (properties[i].property_status & PROPERTY_STATUS_NEEDS_PULL || properties[i].property_status & PROPERTY_STATUS_NEEDS_PULL_WAITING) {
        // A property in another WuObject needs this property as it's initial value,
        // but this property is also waiting for its initial value itself (a chain of unitialised properties)
        // So we only mark that the next push needs to be forced, and wait for the arrival of this
        // property's initial value before propagating.
        properties[i].property_status |= PROPERTY_STATUS_FORCE_NEXT_PUSH;
      } else {
        // Otherwise (the property's value is already available), immediately schedule it to be propagated
        properties[i].property_status = (PROPERTY_STATUS_NEEDS_PUSH | PROPERTY_STATUS_FORCE_NEXT_PUSH);
      }
      DEBUGF_WKPF("WKPF: wkpf_property_needs_initialisation_push: (index %x port %x, property %x): value %x, status %x\n", i, wuobject->port_number, property_number, properties[i].value, properties[i].property_status);
      return WKPF_OK;
    }
  }
  return WKPF_ERR_SHOULDNT_HAPPEN;
}

bool inline wkpf_property_status_is_dirty(uint8_t status) {
  if (!((status & PROPERTY_STATUS_NEEDS_PUSH) || (status & PROPERTY_STATUS_NEEDS_PULL)))
    return false; // Doesn't need push or pull so skip.
  if ((status & 0x0C) == 0)
    return true; // Didn't fail, or only once (because bits 2,3 are 0): send message
  uint8_t timer_bit = (nvm_current_time >> (status & PROPERTY_STATUS_FAILURE_COUNT_TIMES2_MASK /* failurecount*2*/)) & 1;
  return (status & 1) == timer_bit;
}

static uint8_t last_returned_dirty_property_index = 0;
bool wkpf_get_next_dirty_property(uint8_t *port_number, uint8_t *property_number, int16_t *value, uint8_t *status) {
  if (last_returned_dirty_property_index >= number_of_properties)
    last_returned_dirty_property_index = number_of_properties-1; // Could happen if wuobjects were removed
  int i = last_returned_dirty_property_index;

  do {
    i = (i+1) % number_of_properties;
    if (wkpf_property_status_is_dirty(properties[i].property_status)) {
      DEBUGF_WKPF("WKPF: wkpf_get_next_dirty_property DIRTY[%x]: port %x property %x status %x\n", i, properties[i].wuobject_port_number, properties[i].property_number, properties[i].property_status);
      last_returned_dirty_property_index = i;
      *port_number = properties[i].wuobject_port_number;
      *property_number = properties[i].property_number;
      *value = properties[i].value;
      *status = properties[i].property_status;
      // Optimistically set this property to not dirty to avoid having to go through the whole list again after the message has been sent.
      // Hopefully succesful propagations will be the most common case.
      // The original status will be passed back through wkpf_propagating_dirty_property_failed in case the propagation fails
      if (properties[i].property_status & PROPERTY_STATUS_NEEDS_PUSH) {
        properties[i].property_status &= (~PROPERTY_STATUS_NEEDS_PUSH & ~PROPERTY_STATUS_FORCE_NEXT_PUSH & 0xF0);
      } else { // PROPERTY_STATUS_NEEDS_PULL: after sending the request, wait for the source node to send the value
        properties[i].property_status &= (~PROPERTY_STATUS_NEEDS_PULL & 0xF0);
        properties[i].property_status |= PROPERTY_STATUS_NEEDS_PULL_WAITING;
      }
      return TRUE;
    }
//    DEBUGF_WKPF("WKPF: wkpf_get_next_dirty_property NOT DIRTY[%x]: port %x property %x status %x\n", i, properties[i].wuobject_port_number, properties[i].property_number, properties[i].property_status);
  } while(i != last_returned_dirty_property_index);
  return FALSE;
}

void wkpf_propagating_dirty_property_failed(uint8_t port_number, uint8_t property_number, uint8_t original_status) {
  uint8_t failure_count = (original_status & PROPERTY_STATUS_FAILURE_COUNT_TIMES2_MASK) / 2;  
  if (failure_count < 7) // increase if failure count < 7
    failure_count++;
  uint8_t timer_bit_number = failure_count*2; 
  uint8_t timer_bit_value = ((nvm_current_time >> timer_bit_number) + 1) & 1; // Add one to flip bit 
  uint8_t new_status = (original_status & 0xF0) | (failure_count << 1) | timer_bit_value;
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].wuobject_port_number == port_number
        && properties[i].property_number == property_number)
      properties[i].property_status = new_status;
  }
  DEBUGF_WKPF("WKPF: wkpf_propagating_dirty_property_failed!!!!! property %x at port %x. previous status %x new status %x\n", property_number, port_number, original_status, new_status);
}

