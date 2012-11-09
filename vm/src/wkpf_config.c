#include <string.h>
#include <avr/eeprom.h>
#include "debug.h"
#include "wkpf.h"
#include "wkpf_config.h"

#define WKPF_FEATURE_ARRAY_SIZE           (WKPF_MAX_FEATURE_NUMBER/8 + 1)

static uint8_t EEMEM eeprom_location_length = 0;
static char EEMEM eeprom_location[LOCATION_MAX_LENGTH] = ""; // Currently can only handle locations that fit into a single message
static uint8_t EEMEM eeprom_wkpf_features[WKPF_FEATURE_ARRAY_SIZE];

#define load_location_length() eeprom_read_byte((u08_t*)&eeprom_location_length)
#define save_location_length(x) eeprom_update_byte((u08_t*)&eeprom_location_length, (u08_t)x)
#define load_location(dest, length) eeprom_read_block((void*)dest, (const void*)eeprom_location, length)
#define save_location(src, length) eeprom_update_block((const void*)src, (void*)eeprom_location, length)
#define feat_addr(feature) &eeprom_wkpf_features[feature / 8]
#define enable_feature(feature) eeprom_update_byte(feat_addr(feature), eeprom_read_byte(feat_addr(feature)) | (1<<(feature % 8)))
#define disable_feature(feature) eeprom_update_byte(feat_addr(feature), eeprom_read_byte(feat_addr(feature)) & ~(1<<(feature % 8)))
#define get_feature_enabled(feature) (eeprom_read_byte(feat_addr(feature)) & (1<<(feature % 8)))

uint8_t wkpf_config_set_location_string(char* src, uint8_t length) {
  if (length > LOCATION_MAX_LENGTH)
    return WKPF_ERR_LOCATION_TOO_LONG;

  save_location_length(length);
  save_location(src, length);
  return WKPF_OK;
}

void wkpf_config_get_location_string(char* dest, uint8_t* length) {
  *length = load_location_length();
  load_location(dest, *length);
}

uint8_t wkpf_config_set_feature_enabled(uint8_t feature, bool enabled) {
  if (feature > WKPF_MAX_FEATURE_NUMBER)
    return WKPF_ERR_UNKNOWN_FEATURE;
  if (enabled)
    enable_feature(feature);
  else
    disable_feature(feature);
  return WKPF_OK;
}

bool wkpf_config_get_feature_enabled(uint8_t feature) {
  return feature <= WKPF_MAX_FEATURE_NUMBER
          && get_feature_enabled(feature) > 0;
}

