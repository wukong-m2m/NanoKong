#ifndef WKPF_PROFILESH
#define WKPF_PROFILESH

#include "types.h"

#define PROPERTY_TYPE_INT       0
#define PROPERTY_TYPE_BOOLEAN   1
#define PROPERTY_ACCESS_RO      (2 << 6)
#define PROPERTY_ACCESS_WO      (1 << 6)
#define PROPERTY_ACCESS_RW      (3 << 6)

typedef struct {
    uint16_t profile_id;
    uint8_t number_of_properties;
    uint8_t[] properties;
    void (*update) (void);
    void *java_object; // TODO: datatype?
} wkpf_profile_definition;

extern uint8_t wkpf_register_profile(wkpf_profile_definition profile);
extern uint8_t wkpf_get_profile_id(int8_t profile_id, **profile);
extern uint8_t wkpf_get_profile_index(int8_t index, **profile);
extern uint8_t wkpf_get_number_of_profiles();


#endif // WKPF_PROFILESH
