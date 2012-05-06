#ifndef WKPF_LINKSH
#define WKPF_LINKSH

#include "types.h"

extern uint8_t wkpf_propagate_property(uint8_t port_number, uint8_t property_number, int16_t value);
extern uint8_t wkpf_get_node_and_port_for_component(uint16_t component_id, address_t *node_id, uint8_t *port_number);
extern uint8_t wkpf_load_component_to_endpoint_map(heap_id_t map_heap_id);
extern uint8_t wkpf_load_links(heap_id_t links_heap_id);

#endif // WKPF_LINKSH
