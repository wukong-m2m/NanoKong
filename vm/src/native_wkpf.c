#include "native.h"
#include "stack.h"
#include "debug.h"
#include "wkpf.h"

void native_wkpf_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_REGISTER_PROFILE) {
    DEBUGF_WKPF("NATIVE_METHOD_REGISTER_PROFILE\n");
    uint8_t number_of_properties = (uint8_t)stack_pop_int();
    uint8_t *properties = (uint8_t *)stack_pop_addr();
    uint16_t profile_id = (uint16_t)stack_pop_int();
    DEBUGF_WKPF("Properties at address %x:\n", properties);
    for(int i=0; i<10; i++) {
      DEBUGF_WKPF("[%x] ", properties[i]);
    }
    DEBUGF_WKPF("\n");
    wkpf_profile_definition profile;
    profile.profile_id = profile_id;
    profile.update = NULL;
    profile.number_of_properties = number_of_properties;
    profile.properties = properties+1; // Seems to be in RAM anyway. This will work while it is, but we want to get it into Flash at some point. +1 to skip the array type byte
    stack_push(wkpf_register_profile(profile));  
  } else if(mref == NATIVE_METHOD_CREATE_ENDPOINT) {
  } else if(mref == NATIVE_METHOD_REMOVE_ENDPOINT) {
  }
}
