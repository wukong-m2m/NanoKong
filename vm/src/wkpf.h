#ifndef WKPF_H
#define WKPF_H

#include "types.h"

#define WKPF_OK                          0
#define WKPF_ERR_ENDPOINT_NOT_FOUND      1
#define WKPF_ERR_PROPERTY_NOT_FOUND      2
#define WKPF_ERR_PROFILE_NOT_FOUND       3
#define WKPF_ERR_READ_ONLY               4
#define WKPF_ERR_WRITE_ONLY              5
#define WKPF_ERR_PORT_IN_USE             6

typedef struct {
    uint16_t node_address;
    uint8_t port_number;
} wkpf_remote_endpoint;

void profile_init();
extern uint8_t wkpf_get_profile_list();

#endif // WKPF_H
