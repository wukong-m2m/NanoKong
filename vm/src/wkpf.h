#ifndef WKPF_H
#define WKPF_H

#include "types.h"

extern int32_t wkpf_get_profile_list();
extern int32_t wkpf_read_property(uint16_t profile_id, u08_t role_id, u08_t property_id);
extern void wkpf_write_property(uint16_t profile_id, u08_t role_id, u08_t property_id, int32_t value);

#endif // WKPF_H
