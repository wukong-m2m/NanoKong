#ifndef WKPF_H
#define WKPF_H

#include "types.h"
#include "wkpf_profiles.h"
#include "wkpf_endpoints.h"
#include "wkpf_properties.h"
#include "wkpf_profile_library.h"

#define WKPF_PROPERTY_TYPE_INT16        0
#define WKPF_PROPERTY_TYPE_BOOLEAN      1
#define WKPF_PROPERTY_ACCESS_READ       (1 << 7)
#define WKPF_PROPERTY_ACCESS_WRITE      (1 << 6)
#define WKPF_PROPERTY_ACCESS_RW         (WKPF_PROPERTY_ACCESS_READ+WKPF_PROPERTY_ACCESS_WRITE)

#define WKPF_IS_READONLY_PROPERTY(x)    ((~x) & WKPF_PROPERTY_ACCESS_WRITE)
#define WKPF_IS_WRITEONLY_PROPERTY(x)   ((~x) & WKPF_PROPERTY_ACCESS_READ)

#define WKPF_OK                                     0
#define WKPF_ERR_ENDPOINT_NOT_FOUND                 1
#define WKPF_ERR_PROPERTY_NOT_FOUND                 2
#define WKPF_ERR_PROFILE_NOT_FOUND                  3
#define WKPF_ERR_READ_ONLY                          4
#define WKPF_ERR_WRITE_ONLY                         5
#define WKPF_ERR_PORT_IN_USE                        6
#define WKPF_ERR_PROFILE_ID_IN_USE                  7
#define WKPF_ERR_OUT_OF_MEMORY                      8
#define WKPF_ERR_WRONG_DATATYPE                     9
#define WKPF_ERR_ENDPOINT_ALREADY_ALLOCATED        10
#define WKPF_ERR_NEED_VIRTUAL_PROFILE_INSTANCE     11
#define WKPF_ERR_SHOULDNT_HAPPEN                 0xFF

extern void wkpf_init();

#endif // WKPF_H
