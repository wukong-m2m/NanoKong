#include "config.h"
#include "types.h"
#include "debug.h"
#include "wkpf.h"
#include "wkpf_properties.h"

typedef struct property_entry_struct {
  uint8_t endpoint_port_number;
  int8_t property_number;
  int16_t value;
  bool is_dirty;
} property_entry;

#define MAX_NUMBER_OF_PROPERTIES 100
uint8_t number_of_properties;
property_entry properties[MAX_NUMBER_OF_PROPERTIES];

uint8_t wkpf_read_property(wkpf_local_endpoint *endpoint, uint8_t property_number, int16_t *value) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].endpoint_port_number == endpoint->port_number && properties[i].property_number == property_number) {
      DEBUGF_WKPF("WKPF read_property: (index %x port %x, property %x): %x\n", i, endpoint->port_number, property_number, properties[i].value);
      *value = properties[i].value;
      return WKPF_OK;
    }
  }
  return WKPF_ERR_SHOULDNT_HAPPEN;
}
uint8_t wkpf_write_property(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, int16_t value) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].endpoint_port_number == endpoint->port_number && properties[i].property_number == property_number) {
      DEBUGF_WKPF("WKPF write_property: (index %x port %x, property %x): %x->%x\n", i, endpoint->port_number, property_number, properties[i].value, value);
      if (properties[i].value != value) {
        properties[i].value = value;
        properties[i].is_dirty = TRUE;
        if (external_access) // Only call update() when someone else writes to the property, not for internal writes (==writes that are already coming from update())
          wkpf_set_need_to_call_update_for_endpoint(endpoint);
      }
      return WKPF_OK;
    }
  }
  return WKPF_ERR_SHOULDNT_HAPPEN;
}

uint8_t wkpf_verify_property(wkpf_local_endpoint *endpoint, uint8_t property_number, uint8_t access, bool external_access, uint8_t type) {
  if (endpoint->profile->number_of_properties <= property_number)
    return WKPF_ERR_PROPERTY_NOT_FOUND;
  uint8_t property = endpoint->profile->properties[property_number];
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

uint8_t wkpf_read_property_int16(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, int16_t *value) {
  uint8_t retval = wkpf_verify_property(endpoint, property_number, WKPF_PROPERTY_ACCESS_READ, external_access, WKPF_PROPERTY_TYPE_INT16);
  if (retval == WKPF_OK)
    return wkpf_read_property(endpoint, property_number, value);
  else
    return retval;
}
uint8_t wkpf_write_property_int16(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, int16_t value) {
  uint8_t retval = wkpf_verify_property(endpoint, property_number, WKPF_PROPERTY_ACCESS_WRITE, external_access, WKPF_PROPERTY_TYPE_INT16);
  if (retval == WKPF_OK)
    return wkpf_write_property(endpoint, property_number, external_access, value);
  else
    return retval;
}

uint8_t wkpf_read_property_boolean(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, bool *value) {
  uint8_t retval = wkpf_verify_property(endpoint, property_number, WKPF_PROPERTY_ACCESS_READ, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);

  if (retval == WKPF_OK) {
    int16_t value_16bit;
    retval = wkpf_read_property(endpoint, property_number, &value_16bit);
    if (retval == WKPF_OK) {
      *value = (int8_t)value_16bit;
    }
    return retval;
  }
  else
    return retval;
}
uint8_t wkpf_write_property_boolean(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, bool value) {
  uint8_t retval = wkpf_verify_property(endpoint, property_number, WKPF_PROPERTY_ACCESS_WRITE, external_access, WKPF_PROPERTY_TYPE_BOOLEAN);
  if (retval == WKPF_OK)
    return wkpf_write_property(endpoint, property_number, external_access, value);
  else
    return retval;
}

uint8_t wkpf_alloc_properties_for_endpoint(wkpf_local_endpoint *endpoint) {
  for (int i=0; i<number_of_properties; i++)
    if (properties[i].endpoint_port_number == endpoint->port_number)
      return WKPF_ERR_ENDPOINT_ALREADY_ALLOCATED;
  if (number_of_properties+endpoint->profile->number_of_properties > MAX_NUMBER_OF_PROPERTIES)
    return WKPF_ERR_OUT_OF_MEMORY;
  DEBUGF_WKPF("WKPF: allocating %x properties for endpoint at port %x\n", endpoint->profile->number_of_properties, endpoint->port_number);
  for (int i=0; i<endpoint->profile->number_of_properties; i++) {
    properties[number_of_properties+i].endpoint_port_number = endpoint->port_number;
    properties[number_of_properties+i].property_number = i;
    properties[number_of_properties+i].value = 0;
    properties[number_of_properties+i].is_dirty = FALSE;
  }
  number_of_properties += endpoint->profile->number_of_properties;
  DEBUGF_WKPF("WKPF: number of properties is now %x\n", number_of_properties);
  return WKPF_OK;
}

uint8_t wkpf_free_properties_for_endpoint(wkpf_local_endpoint *endpoint) {
  int16_t first=-1, last=-1;
  // Find the sequence of properties corresponding to this endpoint
  for (int i=0; i<number_of_properties; i++) {
    if (first==-1 && properties[i].endpoint_port_number==endpoint->port_number)
      first = i;
    if (first!=-1 && properties[i].endpoint_port_number!=endpoint->port_number)
      last = i;
  }
  if (last != -1) // If there are any, and there are other properties following after, shift them down to compact the property list
    while (last<number_of_properties)
      properties[first++] = properties[last++];
  number_of_properties -= endpoint->profile->number_of_properties;
  return WKPF_OK;
}

bool wkpf_any_property_dirty() {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].is_dirty)
      return TRUE;
  }
  return FALSE;
}

bool wkpf_get_next_dirty_property(uint8_t *port_number, uint8_t *property_number) {
  for (int i=0; i<number_of_properties; i++) {
    if (properties[i].is_dirty) {
      properties[i].is_dirty = FALSE;
      DEBUGF_WKPF("DIRTY[%x]: port %x property %x retval %x\n", i, properties[i].endpoint_port_number, properties[i].property_number, ((uint16_t)properties[i].endpoint_port_number)<<8 | properties[i].property_number);
      *port_number = properties[i].endpoint_port_number;
      *property_number = properties[i].property_number;
      return TRUE;
    }
//    DEBUGF_WKPF("NOT DIRTY[%x]: port %x property %x retval %x\n", i, properties[i].endpoint_port_number, properties[i].property_number, ((uint16_t)properties[i].endpoint_port_number)<<8 | properties[i].property_number);
  }
  return FALSE;
}
