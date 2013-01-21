//# vim: ts=2 sw=2
#include "config.h"
#include "types.h"
#include "debug.h"
#include "nvmcomm.h"
#include "heap.h"
#include "array.h"
#include "wkpf.h"
#include "led.h"
#include "group.h"
#include "wkpf_properties.h"
#include "wkpf_links.h"

#define MAX_NUMBER_OF_COMPONENTS 10
uint8_t number_of_components;
remote_endpoints component_to_wuobject_map[MAX_NUMBER_OF_COMPONENTS];

#define MAX_NUMBER_OF_LINKS 10
uint8_t number_of_links;
link_entry links[MAX_NUMBER_OF_LINKS];

bool wkpf_get_component_id(uint8_t port_number, uint16_t *component_id) {
  for(int i=0; i<number_of_components; i++) {
    uint16_t number_of_endpoints = component_to_wuobject_map[i].number_of_endpoints;
    for(int j=0; j<number_of_endpoints; j++) {
      if(component_to_wuobject_map[i].endpoints[j].node_id == nvmcomm_get_node_id()
          && component_to_wuobject_map[i].endpoints[j].port_number == port_number) {
        *component_id = i;
        return true; // Found
      }
    }
  }
  return false; // Not found. Could happen for wuobjects that aren't used in the application (unused sensors, actuators, etc).
}

uint8_t wkpf_get_link_by_dest_property_and_dest_wuclass_id(uint8_t property_number, uint16_t wuclass_id, link_entry *entry) {
  for (int i=0; i<number_of_links; i++) {
    if (links[i].dest_property_number == property_number && links[i].dest_wuclass_id == wuclass_id) {
      *entry = links[i];
      return WKPF_OK;
    }
  }
  return WKPF_ERR_LINK_NOT_FOUND;
}

uint8_t wkpf_load_heartbeat_to_node_map(heap_id_t heartbeat_map_heap_id) {
#ifdef NVM_USE_GROUP
  uint16_t number_of_entries = array_length(heartbeat_map_heap_id)/sizeof(heap_id_t);

  DEBUGF_WKPF("WKPF: Scanning %x heartbeat groups (%x bytes)\n\n", number_of_entries, array_length(heartbeat_map_heap_id));

  /* No restrictions on the size of heartbeat group yet
  if (number_of_entries>MAX_NUMBER_OF_COMPONENTS)
    return WKPF_ERR_OUT_OF_MEMORY;
  */

  for(int i=0; i<number_of_entries; i++) {
    heap_id_t nodes_heap_id = *((uint8_t *)heap_get_addr(heartbeat_map_heap_id)+1+(2*i));
    uint16_t number_of_nodes = array_length(nodes_heap_id)/sizeof(address_t);
    address_t *nodes = (address_t *)((uint8_t *)heap_get_addr(nodes_heap_id)+1); // +1 to skip type byte

    DEBUGF_WKPF("WKPF: Scanning heartbeat group %x with %x nodes\n", number_of_entries, number_of_nodes);
    for (int j=0; j<number_of_nodes; j++) {
      DEBUGF_WKPF("\tnode %x", nodes[j]);
      if (nodes[j] == nvmcomm_get_node_id()) {
        // Daisy chain heartbeat
        if (j == 0) {
          group_add_node_to_watch(nodes[number_of_nodes-1]);
        } else {
          group_add_node_to_watch(nodes[j-1]);
        }
        break;
      }
    }
    DEBUGF_WKPF("\n");
  }

#endif // NVM_USE_GROUP
  return WKPF_OK;
}

uint8_t wkpf_load_component_to_wuobject_map(heap_id_t map_heap_id) {
  uint16_t number_of_entries = array_length(map_heap_id)/sizeof(heap_id_t);

  DEBUGF_WKPF("WKPF: Registering %x components (%x bytes)\n\n", number_of_entries, array_length(map_heap_id));

  if (number_of_entries>MAX_NUMBER_OF_COMPONENTS)
    return WKPF_ERR_OUT_OF_MEMORY;

  for(int i=0; i<number_of_entries; i++) {
    heap_id_t nodes_heap_id = *((uint8_t *)heap_get_addr(map_heap_id)+1+(2*i));
    uint16_t number_of_nodes = array_length(nodes_heap_id)/sizeof(remote_endpoint);
    remote_endpoint *nodes = (remote_endpoint *)((uint8_t *)heap_get_addr(nodes_heap_id)+1); // +1 to skip type byte

    component_to_wuobject_map[i] = (remote_endpoints){number_of_nodes, nodes};
    DEBUGF_WKPF("WKPF: Registered component wuobject: component %x -> at \n", i);
    for (int j=0; j<number_of_nodes; j++) {
      DEBUGF_WKPF("\t (node %x, port %x)\n", nodes[j].node_id, nodes[j].port_number);
    }
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

bool wkpf_does_property_need_initialisation_pull(uint8_t port_number, uint8_t property_number) {
  uint16_t component_id;
  wkpf_get_component_id(port_number, &component_id);
  
  for(int i=0; i<number_of_links; i++) {
    if (links[i].dest_component_id == component_id
        && links[i].dest_property_number == property_number) {
      uint16_t number_of_endpoints = component_to_wuobject_map[links[i].src_component_id].number_of_endpoints;
      for (int j=0; j<number_of_endpoints; j++) {
        if (component_to_wuobject_map[links[i].src_component_id].endpoints[j].node_id == nvmcomm_get_node_id() 
            && wkpf_node_is_leader(links[i].src_component_id, nvmcomm_get_node_id())) {
          return false;
        }
      }
      return true; // There is a link to this property, coming from another node. We need to ask it for the initial value
    } 
  }
  return false;
}

uint8_t wkpf_pull_property(uint8_t port_number, uint8_t property_number) {
  uint16_t component_id;
  wkpf_get_component_id(port_number, &component_id);

  for(int i=0; i<number_of_links; i++) {
    if(links[i].dest_component_id == component_id
        && links[i].dest_property_number == property_number) {
      uint16_t src_component_id = links[i].src_component_id;
      if (src_component_id > number_of_components) {
        DEBUGF_WKPF("WKPF: !!!! ERROR !!!! component id out of range %x\n", src_component_id);
        return WKPF_ERR_SHOULDNT_HAPPEN;
      }
      uint8_t src_property_number = links[i].src_property_number;
      uint8_t src_port_number = wkpf_leader_for_component(src_component_id).port_number;
      address_t src_node_id = wkpf_leader_for_component(src_component_id).node_id;
      if (src_node_id != nvmcomm_get_node_id()) {
        // Properties with local sources will be initialised eventually, so we only need to send a message
        // to ask for initial values coming from remote nodes
        return wkpf_send_request_property_init(src_node_id, src_port_number, src_property_number);      
      }
    }
  }
  return WKPF_ERR_SHOULDNT_HAPPEN;
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
      uint8_t dest_port_number = wkpf_leader_for_component(dest_component_id).port_number;
      address_t dest_node_id = wkpf_leader_for_component(dest_component_id).node_id;
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
  uint8_t wkpf_error_code;
  uint8_t port_number;
  uint8_t property_number;
  int16_t value;
  uint8_t status;
  while(wkpf_get_next_dirty_property(&port_number, &property_number, &value, &status))  {
    nvmcomm_poll(); // Process incoming messages
    if (status & PROPERTY_STATUS_NEEDS_PUSH) {
      wkpf_error_code = wkpf_propagate_property(port_number, property_number, value);
      if (wkpf_error_code == WKPF_OK)
        blink_once(LED4);
    } else { // PROPERTY_STATUS_NEEDS_PULL
      DEBUGF_WKPF("WKPF: (pull) requesting initial value for property %x at port %x\n", property_number, port_number);
      wkpf_error_code = wkpf_pull_property(port_number, property_number);
      if (wkpf_error_code == WKPF_OK)
        blink_twice(LED4);
    }
    if (wkpf_error_code != WKPF_OK) { // TODONR: need better retry mechanism
      DEBUGF_WKPF("WKPF: ------!!!------ Propagating property failed: port %x property %x error %x\n", port_number, property_number, wkpf_error_code);
      wkpf_propagating_dirty_property_failed(port_number, property_number, status);
      blink_thrice(LED4);
      return wkpf_error_code;
    }
  }
  return WKPF_OK;
}

uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, address_t *node_id, uint8_t *port_number) {
  if (component_id > number_of_components)
    return WKPF_ERR_COMPONENT_NOT_FOUND;
  remote_endpoint endpoint;
  if (wkpf_local_endpoint_for_component(component_id, &endpoint) == WKPF_ERR_ENDPOINT_NOT_FOUND)
    return WKPF_ERR_ENDPOINT_NOT_FOUND;
  *node_id = endpoint.node_id;
  *port_number = endpoint.port_number;
  return WKPF_OK;
}

uint8_t wkpf_remove_endpoint_from_component(int index, remote_endpoints* component) {
  if (component == NULL) {
    return WKPF_ERR_COMPONENT_NOT_FOUND;
  }

  for (int c=index; c<component->number_of_endpoints; ++c) {
    component->endpoints[c] = component->endpoints[c+1];
  }
  component->number_of_endpoints--;

#ifdef DEBUG
  DEBUGF_WKPF("Removing endpoint from component");
#endif

  return WKPF_OK;
}

uint8_t wkpf_remove_endpoint_from_component_id(int index, int component_id) {
  if (component_id > MAX_NUMBER_OF_COMPONENTS) {
    return WKPF_ERR_COMPONENT_NOT_FOUND;
  }

  for (int c=index; c<component_to_wuobject_map[component_id].number_of_endpoints; ++c) {
    component_to_wuobject_map[component_id].endpoints[c] = component_to_wuobject_map[component_id].endpoints[c+1];
  }
  component_to_wuobject_map[component_id].number_of_endpoints--;

#ifdef DEBUG
  DEBUGF_WKPF("Removing endpoint from component id %x", component_id);
#endif

  return WKPF_OK;
}

uint8_t wkpf_insert_endpoint_for_component(remote_endpoint endpoint, uint8_t position, remote_endpoints* component) {
    if (component == NULL) {
        return WKPF_ERR_COMPONENT_NOT_FOUND;
    }

    // Alloc more memory
    component->endpoints = (remote_endpoint*)realloc(component->endpoints, sizeof(remote_endpoint) * (component->number_of_endpoints+1));

    for (int c=component->number_of_endpoints; c>position; ++c) {
        component->endpoints[c] = component->endpoints[c-1];
    }
    component->endpoints[position] = endpoint;
    component->number_of_endpoints++;

    return WKPF_OK;
}

uint8_t wkpf_insert_endpoint_for_component_id(remote_endpoint endpoint, uint8_t position, int component_id) {
    if (component_id > MAX_NUMBER_OF_COMPONENTS) {
        return WKPF_ERR_COMPONENT_NOT_FOUND;
    }

    // Alloc more memory
    component_to_wuobject_map[component_id].endpoints = (remote_endpoint*)realloc(component_to_wuobject_map[component_id].endpoints, sizeof(remote_endpoint) * (component_to_wuobject_map[component_id].number_of_endpoints+1));

    for (int c=component_to_wuobject_map[component_id].number_of_endpoints; c>position; ++c) {
        component_to_wuobject_map[component_id].endpoints[c] = component_to_wuobject_map[component_id].endpoints[c-1];
    }
    component_to_wuobject_map[component_id].endpoints[position] = endpoint;
    component_to_wuobject_map[component_id].number_of_endpoints++;

    return WKPF_OK;
}

bool wkpf_node_is_leader(uint16_t component_id, address_t node_id) {
  return component_to_wuobject_map[component_id].number_of_endpoints > 0
    && component_to_wuobject_map[component_id].endpoints[0].node_id == node_id;
}

remote_endpoint wkpf_leader_for_component(uint16_t component_id) {
  return component_to_wuobject_map[component_id].endpoints[0];
}

bool wkpf_get_component_for_node(address_t node_id, int component_id, remote_endpoints** component) {
  if (component_id > MAX_NUMBER_OF_COMPONENTS) {
    return false;
  }

  for (int j=0; j<component_to_wuobject_map[component_id].number_of_endpoints; ++j) {
    if (component_to_wuobject_map[component_id].endpoints[j].node_id == node_id) {
      *component = component_to_wuobject_map + component_id;
      return true;
    }
  }

  return false;
}

uint8_t wkpf_endpoints_for_component(uint16_t component_id, remote_endpoint* endpoints) {
  if (component_id > MAX_NUMBER_OF_COMPONENTS) {
    return WKPF_ERR_COMPONENT_NOT_FOUND;
  }

  endpoints = component_to_wuobject_map[component_id].endpoints;
  return WKPF_OK;
}

uint8_t wkpf_local_endpoint_for_component(uint16_t component_id, remote_endpoint* endpoint) {
  for (int i=0; i<component_to_wuobject_map[component_id].number_of_endpoints; i++) {
    if (component_to_wuobject_map[component_id].endpoints[i].node_id == nvmcomm_get_node_id())
      *endpoint = component_to_wuobject_map[component_id].endpoints[i];
      return WKPF_OK;
  }

  return WKPF_ERR_ENDPOINT_NOT_FOUND;
}

