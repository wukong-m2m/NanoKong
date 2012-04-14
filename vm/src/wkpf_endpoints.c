#include "config.h"
#include "types.h"
#include "debug.h"
#include "heap.h"
#include "wkpf.h"
#include "wkpf_endpoints.h"
#include "wkpf_properties.h"

#define MAX_NUMBER_OF_ENDPOINTS 10
uint8_t number_of_endpoints;
wkpf_local_endpoint endpoints[MAX_NUMBER_OF_ENDPOINTS];

uint8_t wkpf_create_endpoint(uint16_t profile_id, uint8_t port_number, heap_id_t virtual_profile_instance_heap_id) {
  uint8_t retval;
  wkpf_profile_definition *profile;

  if (number_of_endpoints == MAX_NUMBER_OF_ENDPOINTS) {
    DEBUGF_WKPF("WKPF: out of memory while creating endpoint for profile %x at port: FAILED\n", profile->profile_id, port_number);
    return WKPF_ERR_OUT_OF_MEMORY;
  }
  
  for (int8_t i=0; i<number_of_endpoints; i++) {
    if (endpoints[i].port_number == port_number) {
      DEBUGF_WKPF("WKPF: port %x in use while creating endpoint for profile id %x: FAILED\n", port_number, profile->profile_id);
      return WKPF_ERR_PORT_IN_USE;
    }
  }

  retval = wkpf_get_profile_by_id(profile_id, &profile);
  if (retval != WKPF_OK)
    return retval;

  if (WKPF_IS_VIRTUAL_PROFILE(profile) && virtual_profile_instance_heap_id==0)
    return WKPF_ERR_NEED_VIRTUAL_PROFILE_INSTANCE;

  endpoints[number_of_endpoints].profile = profile;
  endpoints[number_of_endpoints].port_number = port_number;
  endpoints[number_of_endpoints].virtual_profile_instance_heap_id = virtual_profile_instance_heap_id;
  retval = wkpf_alloc_properties_for_endpoint(&endpoints[number_of_endpoints]);
  if (retval != WKPF_OK)
    return retval;
  DEBUGF_WKPF("WKPF: creating endpoint for profile id %x at port %x\n", profile->profile_id, port_number);
  // Run update function once to initialise properties.
  wkpf_need_to_call_update_for_endpoint(&endpoints[number_of_endpoints]);

  number_of_endpoints++;
  return WKPF_OK;
}

uint8_t wkpf_remove_endpoint(uint8_t port_number) {
  for (int8_t i=0; i<number_of_endpoints; i++) {
    if (endpoints[i].port_number == port_number) {
      // Endpoint at index i will be removed:
      // Free allocated properties
      wkpf_free_properties_for_endpoint(&endpoints[i]);
      // Move endpoints at higher indexes on index down
      for (int8_t j=i+1; j<number_of_endpoints; j++) {
        endpoints[j-1] = endpoints[j];
      }
      number_of_endpoints--;
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: no endpoint at port %x found: FAILED\n", port_number);
  return WKPF_ERR_ENDPOINT_NOT_FOUND;  
}

uint8_t wkpf_get_endpoint_by_port(uint8_t port_number, wkpf_local_endpoint **endpoint) {
  for (int8_t i=0; i<number_of_endpoints; i++) {
    if (endpoints[i].port_number == port_number) {
      *endpoint = &endpoints[i];
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: no endpoint at port %x found: FAILED\n", port_number);
  return WKPF_ERR_ENDPOINT_NOT_FOUND;
}

uint8_t wkpf_get_endpoint_by_index(uint8_t index, wkpf_local_endpoint **endpoint) {
  if (index >= number_of_endpoints) {
    DEBUGF_WKPF("WKPF: no endpoint at index %x found: FAILED\n", index);
    return WKPF_ERR_ENDPOINT_NOT_FOUND;
  }
  *endpoint = &endpoints[index];
  return WKPF_OK;  
}

uint8_t wkpf_get_endpoint_by_heap_id(heap_id_t virtual_profile_instance_heap_id, wkpf_local_endpoint **endpoint) {
  for (int8_t i=0; i<number_of_endpoints; i++) {
    if (endpoints[i].virtual_profile_instance_heap_id == virtual_profile_instance_heap_id) {
      *endpoint = &endpoints[i];
      return WKPF_OK;
    }
  }
  DEBUGF_WKPF("WKPF: no endpoint for heap_id %x found: FAILED\n", virtual_profile_instance_heap_id);
  return WKPF_ERR_ENDPOINT_NOT_FOUND;
}

uint8_t wkpf_get_number_of_endpoints() {
  return number_of_endpoints;
}

void wkpf_need_to_call_update_for_endpoint(wkpf_local_endpoint *endpoint) {
  // TODONR: for now just call directly for native profiles
  // Java update should be handled by returning from the WKPF.select() function
  if (WKPF_IS_NATIVE_ENDPOINT(endpoint))
    endpoint->profile->update(endpoint);
}

bool wkpf_heap_id_in_use(heap_id_t heap_id) {
 // To prevent virtual profile objects from being garbage collected  
  for (int i=0; i<number_of_endpoints; i++)
    if (endpoints[i].virtual_profile_instance_heap_id == heap_id)
      return TRUE;
  return FALSE;
}


