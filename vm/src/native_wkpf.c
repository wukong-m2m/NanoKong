#include "native.h"
#include "stack.h"
#include "debug.h"
#include "wkpf.h"

#include "heap.h"

void native_wkpf_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_REGISTER_PROFILE) {
    uint8_t number_of_properties = (uint8_t)stack_pop_int();
    uint8_t *properties = (uint8_t *)stack_pop_addr();
    uint16_t profile_id = (uint16_t)stack_pop_int();
    wkpf_profile_definition profile;
    profile.profile_id = profile_id;
    profile.update = NULL;
    profile.number_of_properties = number_of_properties;
    profile.properties = properties+1; // Seems to be in RAM anyway. This will work while it is, but we want to get it into Flash at some point. +1 to skip the array type byte
    DEBUGF_WKPF("WKPF: Registering virtual profile with id %x\n", profile_id);
    stack_push(wkpf_register_profile(profile));  
  } else if(mref == NATIVE_METHOD_CREATE_ENDPOINT) {
    heap_id_t virtual_profile_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    uint8_t port_number = (uint8_t)stack_pop_int();
    uint16_t profile_id = (uint16_t)stack_pop_int();
    DEBUGF_WKPF("WKPF: Creating endpoint for virtual profile with id %x at port %x (heap_id: %x)\n", profile_id, port_number, virtual_profile_instance_heap_id);
    stack_push(wkpf_create_endpoint(profile_id, port_number, virtual_profile_instance_heap_id));
  } else if(mref == NATIVE_METHOD_REMOVE_ENDPOINT) {
    uint8_t port_number = (uint8_t)stack_pop_int();
    DEBUGF_WKPF("WKPF: Removing endpoint at port %x\n", port_number);
    stack_push(wkpf_remove_endpoint(port_number));
  }
}
