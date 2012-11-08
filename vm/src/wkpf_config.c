#include <string.h>
#include <avr/eeprom.h>
#include "wkpf.h"
#include "wkpf_config.h"

static uint8_t EEMEM eeprom_location_length = 0;
static char EEMEM eeprom_location[LOCATION_MAX_LENGTH] = ""; // Currently can only handle locations that fit into a single message

#define load_location_length() eeprom_read_byte((u08_t*)&eeprom_location_length)
#define save_location_length(x) eeprom_update_byte((u08_t*)&eeprom_location_length, (u08_t)x)
#define load_location(dest) eeprom_read_block((void*)dest, (const void*)eeprom_location, LOCATION_MAX_LENGTH)
#define save_location(src) eeprom_update_block((const void*)src, (void*)eeprom_location, LOCATION_MAX_LENGTH)

uint8_t wkpf_config_set_location_string(char* src, uint8_t length) {
  if (length > LOCATION_MAX_LENGTH)
    return WKPF_ERR_LOCATION_TOO_LONG;

  save_location_length(length);
  save_location(src);
  return WKPF_OK;
}

void wkpf_config_get_location_string(char* dest, uint8_t* length) {
  load_location(dest);
  *length = load_location_length();
}


