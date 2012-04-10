#ifndef WKPF_PROPERTYSTOREH
#define WKPF_PROPERTYSTOREH

#include "types.h"

extern int8_t wkpf_read_property_int32(uint8_t port_number, uint8_t property_number, int32_t *value);
extern int8_t wkpf_write_property_int32(uint8_t port_number, uint8_t property_number, int32_t value);
extern int8_t wkpf_read_property_int8(uint8_t port_number, uint8_t property_number, int32_t *value);
extern int8_t wkpf_write_property_int8(uint8_t port_number, uint8_t property_number, int32_t value);

#endif // WKPF_PROPERTYSTOREH
