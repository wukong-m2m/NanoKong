#include "config.h"
#include "types.h"
#include "debug.h"
#include "wkpf.h"
#include "wkpf_profiles.h"

uint8_t number_of_profiles;
wkpf_profile_definition profiles[MAX_NUMBER_OF_PROFILES];

uint8_t wkpf_register_profile(wkpf_profile_definition profile) {
  if (number_of_profiles == MAX_NUMBER_OF_PROFILES) {
    DEBUGF_WKPF("WKPF: out of memory while registering profile id %x: FAILED\n", profile.profile_id);
    return WKPF_ERR_OUT_OF_MEMORY;
  }
  for (int8_t i=0; i<number_of_profiles; i++) {
    if (profiles[i].profile_id == profile.profile_id) {
      DEBUGF_WKPF("WKPF: profile id in use while registering profile id %x: FAILED\n", profile.profile_id);
      return WKPF_ERR_PROFILE_ID_IN_USE;
    }
  }
  DEBUGF_WKPF("WKPF: registering profile id %x at index %x\n", profile.profile_id, number_of_profiles);
  profiles[number_of_profiles] = profile;
  number_of_profiles++;
  return WKPF_OK;
}

uint8_t wkpf_get_profile_by_id(uint16_t profile_id, wkpf_profile_definition **profile) {
  for (int8_t i=0; i<number_of_profiles; i++) {
    if (profiles[i].profile_id == profile_id) {
      *profile = &profiles[i];      
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: not profile with id %x found: FAILED\n", profile_id);
  return WKPF_ERR_PROFILE_NOT_FOUND;
}

uint8_t wkpf_get_profile_by_index(uint8_t index, wkpf_profile_definition **profile) {
  if (index >= number_of_profiles) {
    DEBUGF_WKPF("WKPF: no profile at index %x found: FAILED\n", index);
    return WKPF_ERR_PROFILE_NOT_FOUND;
  }
  *profile = &profiles[index];
  return WKPF_OK;
}

uint8_t wkpf_get_number_of_profiles() {
  return number_of_profiles;
}
