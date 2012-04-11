#ifndef WKPF_PROPERTIESH
#define WKPF_PROPERTIESH

#include "types.h"
#include "wkpf_endpoints.h"

extern uint8_t wkpf_read_property_int16(wkpf_local_endpoint *endpoint, uint8_t property_number, int16_t *value);
extern uint8_t wkpf_write_property_int16(wkpf_local_endpoint *endpoint, uint8_t property_number, int16_t value);
extern uint8_t wkpf_read_property_boolean(wkpf_local_endpoint *endpoint, uint8_t property_number, int8_t *value);
extern uint8_t wkpf_write_property_boolean(wkpf_local_endpoint *endpoint, uint8_t property_number, int8_t value);

extern uint8_t wkpf_alloc_properties_for_endpoint(wkpf_local_endpoint *endpoint);
extern uint8_t wkpf_free_properties_for_endpoint(wkpf_local_endpoint *endpoint);

#endif // WKPF_PROPERTIESH
