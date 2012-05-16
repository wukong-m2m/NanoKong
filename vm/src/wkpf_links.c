#include "config.h"
#include "types.h"
#include "debug.h"
#include "nvmcomm.h"
#include "heap.h"
#include "array.h"
#include "wkpf.h"
#include "wkpf_properties.h"

typedef struct remote_endpoint_struct {
  address_t node_id;
  uint8_t port_number;  
} remote_endpoint;

typedef struct link_entry_struct {
  uint16_t src_component_id;
  uint8_t src_property_number;
  uint16_t dest_component_id;  
  uint8_t dest_property_number;
  uint16_t dest_wuclass_id; // This is only here because there is an extra check on wuclass_id when remotely setting properties, but actually that's not strictly necessary. Not sure if it's worth the extra memory, but if we store this in flash it might be ok.
} link_entry;


#define MAX_NUMBER_OF_COMPONENTS 10
uint8_t number_of_components;
remote_endpoint component_to_wuobject_map[MAX_NUMBER_OF_COMPONENTS];

#define MAX_NUMBER_OF_LINKS 10
uint8_t number_of_links;
link_entry links[MAX_NUMBER_OF_LINKS];


bool wkpf_get_component_id(uint8_t port_number, uint16_t *component_id) {
  for(int i=0; i<number_of_components; i++) {
    if(component_to_wuobject_map[i].node_id == nvmcomm_get_node_id()
        && component_to_wuobject_map[i].port_number == port_number) {
      *component_id = i;
      return true; // Found
    }
  }
  return false; // Not found. Could happen for wuobjects that aren't used in the application (unused sensors, actuators, etc).
}

uint8_t wkpf_load_component_to_wuobject_map(heap_id_t map_heap_id) {
  uint16_t number_of_entries = array_length(map_heap_id)/sizeof(remote_endpoint);
  remote_endpoint *map = (remote_endpoint *)((uint8_t *)heap_get_addr(map_heap_id)+1); // +1 to skip type byte

  DEBUGF_WKPF("WKPF: Registering %x components (%x bytes, %x each)\n\n", number_of_entries, array_length(map_heap_id), sizeof(remote_endpoint));

  if (number_of_entries>MAX_NUMBER_OF_COMPONENTS)
    return WKPF_ERR_OUT_OF_MEMORY;
  for(int i=0; i<number_of_entries; i++) {
    component_to_wuobject_map[i] = map[i];
    DEBUGF_WKPF("WKPF: Registered component wuobject: component %x -> (node %x, port %x)\n", i, component_to_wuobject_map[i].node_id, component_to_wuobject_map[i].port_number);
  }
  number_of_components = number_of_entries;
  return WKPF_OK;
}

uint8_t wkpf_load_links(heap_id_t links_heap_id) {
  uint16_t number_of_entries = array_length(links_heap_id)/sizeof(link_entry);
  link_entry *links_p = (link_entry *)((uint8_t *)heap_get_addr(links_heap_id)+1); // +1 to skip type byte

  DEBUGF_WKPF("WKPF: Registering %x links (%x bytes, %x each)\n\n", number_of_entries, array_length(links_heap_id), sizeof(link_entry));
  
  if (number_of_entries>MAX_NUMBER_OF_LINKS)
    return WKPF_ERR_OUT_OF_MEMORY;
  for(int i=0; i<number_of_entries; i++) {
    links[i] = links_p[i];
    DEBUGF_WKPF("WKPF: Registered link: (component %x, property %x) -> (component %x, property %x, wuclass %x)\n",
                links[i].src_component_id, links[i].src_property_number,
                links[i].dest_component_id, links[i].dest_property_number,
                links[i].dest_wuclass_id);
  }
  number_of_links = number_of_entries;
  return WKPF_OK;
}

uint8_t wkpf_propagate_property(uint8_t port_number, uint8_t property_number, int16_t value) {
  uint16_t component_id;
  if (!wkpf_get_component_id(port_number, &component_id))
    return WKPF_OK; // WuObject isn't used in the application.
  
  wkpf_local_wuobject *src_wuobject;
  uint8_t wkpf_error_code;

  DEBUGF_WKPF("WKPF: propagate property number %x of component %x on port %x (value %x)\n", property_number, component_id, port_number, value);

  wkpf_get_wuobject_by_port(port_number, &src_wuobject);
  for(int i=0; i<number_of_links; i++) {
    if(links[i].src_component_id == component_id
        && links[i].src_property_number == property_number) {
      uint16_t dest_component_id = links[i].dest_component_id;
      if (dest_component_id > number_of_components) {
        DEBUGF_WKPF("WKPF: !!!! ERROR !!!! component id out of range %x\n", dest_component_id);
        return WKPF_ERR_SHOULDNT_HAPPEN;
      }
      uint8_t dest_property_number = links[i].dest_property_number;
      uint8_t dest_port_number = component_to_wuobject_map[dest_component_id].port_number;
      address_t dest_node_id = component_to_wuobject_map[dest_component_id].node_id;
      if (dest_node_id == nvmcomm_get_node_id()) {
        // Local
        wkpf_local_wuobject *dest_wuobject;
        wkpf_error_code = wkpf_get_wuobject_by_port(dest_port_number, &dest_wuobject);
        if (wkpf_error_code == WKPF_OK) {
          DEBUGF_WKPF("WKPF: propagate_property (local). (%x, %x)->(%x, %x), value %x\n", port_number, property_number, dest_port_number, dest_property_number, value);
          if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_BOOLEAN)
            wkpf_error_code = wkpf_external_write_property_boolean(dest_wuobject, dest_property_number, value);
          else if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_SHORT)
            wkpf_error_code = wkpf_external_write_property_int16(dest_wuobject, dest_property_number, value);
          else
            wkpf_error_code = wkpf_external_write_property_refresh_rate(dest_wuobject, dest_property_number, value);
        }
      } else {
        // Remote
        DEBUGF_WKPF("WKPF: propagate_property (remote). (%x, %x)->(%x, %x, %x), value %x\n", port_number, property_number, dest_node_id, dest_port_number, dest_property_number, value);
        if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_BOOLEAN)
          wkpf_error_code = wkpf_send_set_property_boolean(dest_node_id, dest_port_number, dest_property_number, links[i].dest_wuclass_id, value);
        else if (WKPF_GET_PROPERTY_DATATYPE(src_wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_SHORT)
          wkpf_error_code = wkpf_send_set_property_int16(dest_node_id, dest_port_number, dest_property_number, links[i].dest_wuclass_id, value);
        else
          wkpf_error_code = wkpf_send_set_property_refresh_rate(dest_node_id, dest_port_number, dest_property_number, links[i].dest_wuclass_id, value);
      }
      if (wkpf_error_code != WKPF_OK)
        return wkpf_error_code;
    }
  }
  return WKPF_OK;
}

uint8_t wkpf_propagate_dirty_properties() {
  if (wkpf_any_property_dirty()) { // this call is still here to have a place to mark failed properties as dirty again. should probably be refactored
    uint8_t wkpf_error_code;
    uint8_t port_number;
    uint8_t property_number;
    int16_t value;
    while(wkpf_get_next_dirty_property(&port_number, &property_number, &value)) {
      nvmcomm_poll(); // Process incoming messages
      wkpf_error_code = wkpf_propagate_property(port_number, property_number, value);
      if (wkpf_error_code != WKPF_OK) { // TODONR: need better retry mechanism
        DEBUGF_WKPF("WKPF: ------!!!------ Propagating property failed: port %x property %x error %x\n", port_number, property_number, wkpf_error_code);
        wkpf_propagating_dirty_property_failed(port_number, property_number);
        return wkpf_error_code;
      }
    }
  }
  return WKPF_OK;
}

uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, address_t *node_id, uint8_t *port_number) {
  if (component_id > number_of_components)
    return WKPF_ERR_COMPONENT_NOT_FOUND;
  *node_id = component_to_wuobject_map[component_id].node_id;
  *port_number = component_to_wuobject_map[component_id].port_number;
  return WKPF_OK;
}

