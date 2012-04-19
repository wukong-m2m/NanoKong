#ifndef WKPF_PROPERTIESH
#define WKPF_PROPERTIESH

#include "types.h"
#include "wkpf_endpoints.h"

// Access functions that check r/w access permission, used for external access
#define wkpf_external_read_property_int16(endpoint, property_number, value)      wkpf_read_property_int16(endpoint, property_number, TRUE, value)
#define wkpf_external_write_property_int16(endpoint, property_number, value)     wkpf_write_property_int16(endpoint, property_number, TRUE, value)
#define wkpf_external_read_property_boolean(endpoint, property_number, value)    wkpf_read_property_boolean(endpoint, property_number, TRUE, value)
#define wkpf_external_write_property_boolean(endpoint, property_number, value)   wkpf_write_property_boolean(endpoint, property_number, TRUE, value)

// Access functions that don't check r/w access permission, used by the profiles to access their own properties
#define wkpf_internal_read_property_int16(endpoint, property_number, value)      wkpf_read_property_int16(endpoint, property_number, FALSE, value)
#define wkpf_internal_write_property_int16(endpoint, property_number, value)     wkpf_write_property_int16(endpoint, property_number, FALSE, value)
#define wkpf_internal_read_property_boolean(endpoint, property_number, value)    wkpf_read_property_boolean(endpoint, property_number, FALSE, value)
#define wkpf_internal_write_property_boolean(endpoint, property_number, value)   wkpf_write_property_boolean(endpoint, property_number, FALSE, value)

extern uint8_t wkpf_read_property_int16(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, int16_t *value);
extern uint8_t wkpf_write_property_int16(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, int16_t value);
extern uint8_t wkpf_read_property_boolean(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, bool *value);
extern uint8_t wkpf_write_property_boolean(wkpf_local_endpoint *endpoint, uint8_t property_number, bool external_access, bool value);

extern uint8_t wkpf_alloc_properties_for_endpoint(wkpf_local_endpoint *endpoint);
extern uint8_t wkpf_free_properties_for_endpoint(wkpf_local_endpoint *endpoint);

extern bool wkpf_any_property_dirty();
extern bool wkpf_get_next_dirty_property(uint8_t *port_number, uint8_t *property_number);

#endif // WKPF_PROPERTIESH
