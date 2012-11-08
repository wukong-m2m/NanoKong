#ifndef WKPF_CONFIG_H
#define WKPF_CONFIG_H

#include "types.h"
#include "nvmcomm.h"

#define LOCATION_MAX_LENGTH (NVMCOMM_MESSAGE_SIZE-3)

// Stores the location in EEPROM, or returns WKPF_ERR_LOCATION_TOO_LONG if the string is too long.
extern uint8_t wkpf_config_set_location_string(char* src, uint8_t length);
// Retrieves the location from EEPROM and stores it in dest. Make sure dest is large enough to hold LOCATION_MAX_LENGTH bytes.
// Returns the number of bytes written
extern void wkpf_config_get_location_string(char* dest, uint8_t* length);

#endif // WKPF_CONFIG_H
