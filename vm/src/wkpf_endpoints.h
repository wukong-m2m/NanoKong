#ifndef WKPF_ENDPOINTSH
#define WKPF_ENDPOINTSH

#include "types.h"
#include "wkpf_profiles.h"

typedef struct wkpf_local_endpoint_struct {
    wkpf_profile_definition *profile;
    uint8_t port_number;
    void* virtual_profile_instance; // Set for virtual profiles, NULL for native profiles
} wkpf_local_endpoint;

extern uint8_t wkpf_create_endpoint(uint16_t profile_id, uint8_t port_number, void* virtual_profile_instance);
extern uint8_t wkpf_remove_endpoint(uint8_t port_number);
extern uint8_t wkpf_get_endpoint_by_port(uint8_t port_number, wkpf_local_endpoint **endpoint);
extern uint8_t wkpf_get_endpoint_by_index(uint8_t index, wkpf_local_endpoint **endpoint);
extern uint8_t wkpf_get_number_of_endpoints();
extern void wkpf_need_to_call_update_for_endpoint(wkpf_local_endpoint *endpoint);

#endif // WKPF_ENDPOINTSH
