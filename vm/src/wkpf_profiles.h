#ifndef WKPF_PROFILESH
#define WKPF_PROFILESH

#include "types.h"

#define WKPF_IS_NATIVE_PROFILE(x)               (x->update != NULL)
#define WKPF_IS_VIRTUAL_PROFILE(x)              (x->update == NULL)

struct wkpf_local_endpoint_struct;
typedef void (*update_function_t)(struct wkpf_local_endpoint_struct *);

typedef struct wkpf_profile_definition {
    uint16_t profile_id;
    update_function_t update; // Set for native profiles, NULL for virtual profiles
    uint8_t number_of_properties;
    uint8_t *properties;
} wkpf_profile_definition;

extern uint8_t wkpf_register_profile(wkpf_profile_definition profile);
extern uint8_t wkpf_get_profile_by_id(uint16_t profile_id, wkpf_profile_definition **profile);
extern uint8_t wkpf_get_profile_by_index(uint8_t index, wkpf_profile_definition **profile);
extern uint8_t wkpf_get_number_of_profiles();

#endif // WKPF_PROFILESH
