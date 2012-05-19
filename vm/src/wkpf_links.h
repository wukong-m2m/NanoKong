#ifndef WKPF_LINKSH
#define WKPF_LINKSH

#include "types.h"

extern uint8_t wkpf_load_component_to_wuobject_map(heap_id_t map_heap_id);
extern uint8_t wkpf_load_links(heap_id_t links_heap_id);
extern bool wkpf_does_property_need_initialisation_pull(uint8_t port_number, uint8_t property_number);
extern uint8_t wkpf_propagate_dirty_properties();
extern uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, address_t *node_id, uint8_t *port_number);

#endif // WKPF_LINKSH
