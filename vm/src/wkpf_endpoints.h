#ifndef WKPF_ENDPOINTSH
#define WKPF_ENDPOINTSH

#include "types.h"
#include "heap.h"
#include "wkpf_profiles.h"

typedef struct wkpf_local_endpoint_struct {
    wkpf_profile_definition *profile;
    uint8_t port_number;
    heap_id_t virtual_profile_instance_heap_id; // Set for virtual profiles, 0 for native profiles
} wkpf_local_endpoint;

extern uint8_t wkpf_create_endpoint(uint16_t profile_id, uint8_t port_number, heap_id_t virtual_profile_instance_heap_id);
extern uint8_t wkpf_remove_endpoint(uint8_t port_number);
extern uint8_t wkpf_get_endpoint_by_port(uint8_t port_number, wkpf_local_endpoint **endpoint);
extern uint8_t wkpf_get_endpoint_by_index(uint8_t index, wkpf_local_endpoint **endpoint);
extern uint8_t wkpf_get_number_of_endpoints();
extern void wkpf_need_to_call_update_for_endpoint(wkpf_local_endpoint *endpoint);
extern bool wkpf_heap_id_in_use(heap_id_t heap_id); // To prevent virtual profile objects from being garbage collected

#endif // WKPF_ENDPOINTSH
