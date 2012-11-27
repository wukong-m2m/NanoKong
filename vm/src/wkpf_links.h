#ifndef WKPF_LINKSH
#define WKPF_LINKSH

#include "types.h"

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

extern uint8_t wkpf_load_component_to_wuobject_map(heap_id_t map_heap_id);
extern uint8_t wkpf_load_links(heap_id_t links_heap_id);
extern bool wkpf_does_property_need_initialisation_pull(uint8_t port_number, uint8_t property_number);
extern uint8_t wkpf_propagate_dirty_properties();
extern uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, address_t *node_id, uint8_t *port_number);
extern bool wkpf_get_component_id(uint8_t port_number, uint16_t *component_id);
extern uint8_t wkpf_get_link_by_dest_property_and_dest_wuclass_id(uint8_t property_number, uint16_t wuclass_id, link_entry *entry);

extern bool wkpf_node_is_leader(uint16_t component_id, address_t node_id);
extern remote_endpoint wkpf_leader_for_component(uint16_t component_id);
extern uint8_t wkpf_local_endpoint_for_component(uint16_t component_id, remote_endpoint* endpoint);

#endif // WKPF_LINKSH
