#include "native.h"
#include "stack.h"
#include "debug.h"
#include "wkpf.h"
#include "nvmcomm.h"

#include "heap.h"

uint8_t wkpf_error_code = 0;

uint8_t wkpf_dirty_property_port_number;
uint8_t wkpf_dirty_property_number;

void native_wkpf_invoke(u08_t mref) {
  
  if(mref == NATIVE_WKPF_METHOD_GETERRORCODE) {
    stack_push(wkpf_error_code);

  } else if(mref == NATIVE_WKPF_METHOD_REGISTER_PROFILE) {
    uint8_t number_of_properties = (uint8_t)stack_pop_int();
    uint8_t *properties = (uint8_t *)stack_pop_addr();
    uint16_t profile_id = (uint16_t)stack_pop_int();
    wkpf_profile_definition profile;
    profile.profile_id = profile_id;
    profile.update = NULL;
    profile.number_of_properties = number_of_properties;
    profile.properties = properties+1; // Seems to be in RAM anyway. This will work while it is, but we want to get it into Flash at some point. +1 to skip the array type byte
    DEBUGF_WKPF("WKPF: Registering virtual profile with id %x\n", profile_id);
    wkpf_error_code = wkpf_register_profile(profile);  

  } else if(mref == NATIVE_WKPF_METHOD_CREATE_ENDPOINT) {
    heap_id_t virtual_profile_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    uint8_t port_number = (uint8_t)stack_pop_int();
    uint16_t profile_id = (uint16_t)stack_pop_int();
    DEBUGF_WKPF("WKPF: Creating endpoint for virtual profile with id %x at port %x (heap_id: %x)\n", profile_id, port_number, virtual_profile_instance_heap_id);
    wkpf_error_code = wkpf_create_endpoint(profile_id, port_number, virtual_profile_instance_heap_id);

  } else if(mref == NATIVE_WKPF_METHOD_REMOVE_ENDPOINT) {
    uint8_t port_number = (uint8_t)stack_pop_int();
    DEBUGF_WKPF("WKPF: Removing endpoint at port %x\n", port_number);
    wkpf_error_code = wkpf_remove_endpoint(port_number);

  } else if(mref == NATIVE_WKPF_METHOD_GETPROPERTYSHORT) {
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_profile_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_endpoint *endpoint;
    wkpf_error_code = wkpf_get_endpoint_by_heap_id(virtual_profile_instance_heap_id, &endpoint);
    if (wkpf_error_code == WKPF_OK) {
      int16_t value;
      wkpf_error_code = wkpf_internal_read_property_int16(endpoint, property_number, &value);
      if (wkpf_error_code == WKPF_OK)
        stack_push(value);
    }
  } else if(mref == NATIVE_WKPF_METHOD_SETPROPERTYSHORT) {
    int16_t value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_profile_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_endpoint *endpoint;
    wkpf_error_code = wkpf_get_endpoint_by_heap_id(virtual_profile_instance_heap_id, &endpoint);
    if (wkpf_error_code == WKPF_OK) {
      wkpf_error_code = wkpf_internal_write_property_int16(endpoint, property_number, value);
    }
  } else if(mref == NATIVE_WKPF_METHOD_GETPROPERTYBOOLEAN) {
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_profile_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_endpoint *endpoint;
    wkpf_error_code = wkpf_get_endpoint_by_heap_id(virtual_profile_instance_heap_id, &endpoint);
    if (wkpf_error_code == WKPF_OK) {
      bool value;
      wkpf_error_code = wkpf_internal_read_property_boolean(endpoint, property_number, &value);
      if (wkpf_error_code == WKPF_OK)
        stack_push(value);
    }
  } else if(mref == NATIVE_WKPF_METHOD_SETPROPERTYBOOLEAN) {
    bool value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_profile_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_endpoint *endpoint;
    wkpf_error_code = wkpf_get_endpoint_by_heap_id(virtual_profile_instance_heap_id, &endpoint);
    if (wkpf_error_code == WKPF_OK) {
      wkpf_error_code = wkpf_internal_write_property_boolean(endpoint, property_number, value);
    }
  } else if (mref == NATIVE_WKPF_METHOD_SETPROPERTYSHORT_REMOTE) {
    int16_t value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    uint8_t port_number = (uint8_t)stack_pop_int();
    int16_t node_id = (int16_t)stack_pop_int();
    address_t my_node_id = nvmcomm_get_node_id();
    if (node_id == my_node_id) {
      wkpf_local_endpoint *endpoint;
      wkpf_error_code = wkpf_get_endpoint_by_port(port_number, &endpoint);
      if (wkpf_error_code == WKPF_OK) {
        DEBUGF_WKPF("Local property update from propertyDispatch. Port %x, property %x, value %x\n", port_number, property_number, value);
        wkpf_error_code = wkpf_external_write_property_int16(endpoint, property_number, value);
      }
    } else {
      DEBUGF_WKPF("Remote property updates not yet implemented.\n");
      wkpf_error_code = WKPF_OK;
    }
  } else if (mref == NATIVE_WKPF_METHOD_SETPROPERTYBOOLEAN_REMOTE) {
    bool value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    uint8_t port_number = (uint8_t)stack_pop_int();
    int16_t node_id = (int16_t)stack_pop_int();
    address_t my_node_id = nvmcomm_get_node_id();
    if (node_id == my_node_id) {
      wkpf_local_endpoint *endpoint;
      wkpf_error_code = wkpf_get_endpoint_by_port(port_number, &endpoint);
      if (wkpf_error_code == WKPF_OK) {
        DEBUGF_WKPF("Local property update from propertyDispatch. Port %x, property %x, value %x\n", port_number, property_number, value);
        wkpf_error_code = wkpf_external_write_property_boolean(endpoint, property_number, value);
      }
    } else {
      DEBUGF_WKPF("Remote property updates not yet implemented.\n");
      wkpf_error_code = WKPF_OK;
    }
  } else if (mref == NATIVE_WKPF_METHOD_SELECT) {
    uint8_t number_of_endpoints = wkpf_get_number_of_endpoints();
    for (int i=0; i<number_of_endpoints; i++) {
      wkpf_local_endpoint *endpoint;
      wkpf_get_endpoint_by_index(i, &endpoint);
      if (endpoint->need_to_call_update) {
        endpoint->need_to_call_update = FALSE;
        stack_push(endpoint->virtual_profile_instance_heap_id | NVM_TYPE_MASK);
        DEBUGF_WKPF("WKPF: WKPF.select returning profile at port %x.\n", endpoint->port_number);
        return;
      }
    }
    DEBUGF_WKPF("WKPF: WKPF.select didn't find any profile to return\n");
    stack_push(0);
  } else if (mref == NATIVE_WKPF_METHOD_GETMYNODEID) {
    stack_push(nvmcomm_get_node_id());    
  } else if (mref == NATIVE_WKPF_METHOD_LOADNEXTDIRTYPROPERTY) {
    stack_push(wkpf_get_next_dirty_property(&wkpf_dirty_property_port_number, &wkpf_dirty_property_number));
  } else if (mref == NATIVE_WKPF_METHOD_GETDIRTYPROPERTYPORTNUMBER) {
    stack_push(wkpf_dirty_property_port_number);
  } else if (mref == NATIVE_WKPF_METHOD_GETDIRTYPROPERTYNUMBER) {
    stack_push(wkpf_dirty_property_number);
  } else if (mref == NATIVE_WKPF_METHOD_GETDIRTYPROPERTYSHORTVALUE) {
    wkpf_local_endpoint *endpoint;
    wkpf_error_code = wkpf_get_endpoint_by_port(wkpf_dirty_property_port_number, &endpoint);
    if (wkpf_error_code == WKPF_OK) {
      int16_t value;
      wkpf_error_code = wkpf_internal_read_property_int16(endpoint, wkpf_dirty_property_number, &value);
      if (wkpf_error_code == WKPF_OK)
        stack_push(value);
    }
  } else if (mref == NATIVE_WKPF_METHOD_GETDIRTYPROPERTYBOOLEANVALUE) {
    wkpf_local_endpoint *endpoint;
    wkpf_error_code = wkpf_get_endpoint_by_port(wkpf_dirty_property_port_number, &endpoint);
    if (wkpf_error_code == WKPF_OK) {
      bool value;
      wkpf_error_code = wkpf_internal_read_property_boolean(endpoint, wkpf_dirty_property_number, &value);
      if (wkpf_error_code == WKPF_OK)
        stack_push(value);
    }
  }
}
