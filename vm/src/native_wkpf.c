#include "native.h"
#include "stack.h"
#include "debug.h"
#include "nvmcomm.h"
#include "heap.h"
#include "array.h"
#include "wkpf.h"
#include "vm.h"
#include "group.h"
#include "avr/native_avr.h"

uint8_t wkpf_error_code = WKPF_OK;

void native_wkpf_invoke(u08_t mref) {
  
  if(mref == NATIVE_WKPF_METHOD_GETERRORCODE) {
    stack_push(wkpf_error_code);

  } else if(mref == NATIVE_WKPF_METHOD_REGISTER_WUCLASS) {
    heap_id_t properties_heap_id = stack_peek(0) & ~NVM_TYPE_MASK;
    uint8_t *properties = (uint8_t *)stack_pop_addr();
    uint16_t wuclass_id = (uint16_t)stack_pop_int();
    wkpf_wuclass_definition wuclass;
    wuclass.wuclass_id = wuclass_id;
    wuclass.update = NULL;
    wuclass.number_of_properties = array_length(properties_heap_id);
    wuclass.properties = properties+1; // Seems to be in RAM anyway. This will work while it is, but we want to get it into Flash at some point. +1 to skip the array type byte
    DEBUGF_WKPF("WKPF: Registering virtual wuclass with id %x\n", wuclass_id);
    wkpf_error_code = wkpf_register_wuclass(wuclass);  

  } else if(mref == NATIVE_WKPF_METHOD_CREATE_WUOBJECT) {
    heap_id_t virtual_wuclass_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    uint8_t port_number = (uint8_t)stack_pop_int();
    uint16_t wuclass_id = (uint16_t)stack_pop_int();
    DEBUGF_WKPF("WKPF: Creating wuobject for virtual wuclass with id %x at port %x (heap_id: %x)\n", wuclass_id, port_number, virtual_wuclass_instance_heap_id);
    wkpf_error_code = wkpf_create_wuobject(wuclass_id, port_number, virtual_wuclass_instance_heap_id);

  } else if(mref == NATIVE_WKPF_METHOD_REMOVE_WUOBJECT) {
    uint8_t port_number = (uint8_t)stack_pop_int();
    DEBUGF_WKPF("WKPF: Removing wuobject at port %x\n", port_number);
    wkpf_error_code = wkpf_remove_wuobject(port_number);

  } else if(mref == NATIVE_WKPF_METHOD_GETPROPERTYSHORT) {
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_wuclass_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_wuobject *wuobject;
    wkpf_error_code = wkpf_get_wuobject_by_heap_id(virtual_wuclass_instance_heap_id, &wuobject);
    if (wkpf_error_code == WKPF_OK) {
      int16_t value;
      wkpf_error_code = wkpf_internal_read_property_int16(wuobject, property_number, &value);
      if (wkpf_error_code == WKPF_OK)
        stack_push(value);
    }
    
  } else if(mref == NATIVE_WKPF_METHOD_SETPROPERTYSHORT) {
    int16_t value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_wuclass_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_wuobject *wuobject;
    wkpf_error_code = wkpf_get_wuobject_by_heap_id(virtual_wuclass_instance_heap_id, &wuobject);
    if (wkpf_error_code == WKPF_OK) {
      wkpf_error_code = wkpf_internal_write_property_int16(wuobject, property_number, value);
    }
    
  } else if(mref == NATIVE_WKPF_METHOD_GETPROPERTYBOOLEAN) {
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_wuclass_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_wuobject *wuobject;
    wkpf_error_code = wkpf_get_wuobject_by_heap_id(virtual_wuclass_instance_heap_id, &wuobject);
    if (wkpf_error_code == WKPF_OK) {
      bool value;
      wkpf_error_code = wkpf_internal_read_property_boolean(wuobject, property_number, &value);
      if (wkpf_error_code == WKPF_OK)
        stack_push(value);
    }
    
  } else if(mref == NATIVE_WKPF_METHOD_SETPROPERTYBOOLEAN) {
    bool value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    heap_id_t virtual_wuclass_instance_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_local_wuobject *wuobject;
    wkpf_error_code = wkpf_get_wuobject_by_heap_id(virtual_wuclass_instance_heap_id, &wuobject);
    if (wkpf_error_code == WKPF_OK) {
      wkpf_error_code = wkpf_internal_write_property_boolean(wuobject, property_number, value);
    }
    
  } else if (mref == NATIVE_WKPF_METHOD_SETPROPERTYSHORT_COMPONENT) {
    int16_t value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    uint16_t component_id = (uint16_t)stack_pop_int();
    address_t node_id;
    uint8_t port_number;
    wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);
    if (wkpf_error_code == WKPF_OK) {
      if (node_id != nvmcomm_get_node_id())
        wkpf_error_code = WKPF_ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED;
      else {
        wkpf_local_wuobject *wuobject;
        wkpf_error_code = wkpf_get_wuobject_by_port(port_number, &wuobject);
        if (wkpf_error_code == WKPF_OK) {
          DEBUGF_WKPF("WKPF: setPropertyShort (local). Port %x, property %x, value %x\n", port_number, property_number, value);
          wkpf_error_code = wkpf_external_write_property_int16(wuobject, property_number, value);
        }
      }
    }

  } else if (mref == NATIVE_WKPF_METHOD_SETPROPERTYBOOLEAN_COMPONENT) {
    bool value = (bool)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    uint16_t component_id = (uint16_t)stack_pop_int();
    address_t node_id;
    uint8_t port_number;
    wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);
    if (wkpf_error_code == WKPF_OK) {
      if (node_id != nvmcomm_get_node_id())
        wkpf_error_code = WKPF_ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED;
      else {
        wkpf_local_wuobject *wuobject;
        wkpf_error_code = wkpf_get_wuobject_by_port(port_number, &wuobject);
        if (wkpf_error_code == WKPF_OK) {
          DEBUGF_WKPF("WKPF: setPropertyBoolean (local). Port %x, property %x, value %x\n", port_number, property_number, value);
          wkpf_error_code = wkpf_external_write_property_boolean(wuobject, property_number, value);
        }
      }
    }

  } else if (mref == NATIVE_WKPF_METHOD_SETPROPERTYREFRESHRATE_COMPONENT) {
    int16_t value = (int16_t)stack_pop_int();
    uint8_t property_number = (uint8_t)stack_pop_int();
    uint16_t component_id = (uint16_t)stack_pop_int();
    address_t node_id;
    uint8_t port_number;
    wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);
    if (wkpf_error_code == WKPF_OK) {
      if (node_id != nvmcomm_get_node_id())
        wkpf_error_code = WKPF_ERR_REMOTE_PROPERTY_FROM_JAVASET_NOT_SUPPORTED;
      else {
        wkpf_local_wuobject *wuobject;
        wkpf_error_code = wkpf_get_wuobject_by_port(port_number, &wuobject);
        if (wkpf_error_code == WKPF_OK) {
          DEBUGF_WKPF("WKPF: setPropertyRefreshRate (local). Port %x, property %x, value %x\n", port_number, property_number, value);
          wkpf_error_code = wkpf_external_write_property_refresh_rate(wuobject, property_number, value);
        }
      }
    }

  } else if (mref == NATIVE_WKPF_METHOD_SELECT) {
    wkpf_local_wuobject *wuobject;
    while(true) {
      // Process any incoming messages
      nvmcomm_poll();
#ifdef NVM_USE_GROUP
      // Send out a heartbeat message if it's due, and check for failed nodes.
      send_heartbeat();
      handle_failure();
#endif // NVM_USE_GROUP
      if (nvm_runlevel == NVM_RUNLVL_VM) {
        // Propagate any dirty properties
        wkpf_propagate_dirty_properties();
        // Check if any wuobjects need updates
        while(wkpf_get_next_wuobject_to_update(&wuobject)) { // Will call update() for native profiles directly, and return true for virtual profiles requiring an update.
          stack_push(wuobject->virtual_wuclass_instance_heap_id | NVM_TYPE_MASK);
          DEBUGF_WKPF("WKPF: WKPF.select returning wuclass at port %x.\n", wuobject->port_number);
          return;
        }
      }
    }

  } else if (mref == NATIVE_WKPF_METHOD_GETMYNODEID) {
    stack_push(nvmcomm_get_node_id());

  } else if (mref == NATIVE_WKPF_METHOD_LOAD_HEARTBEAT_MAP) {
    heap_id_t map_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_error_code = wkpf_load_heartbeat_to_node_map(map_heap_id);

  } else if (mref == NATIVE_WKPF_METHOD_LOAD_COMPONENT_MAP) {
    heap_id_t map_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_error_code = wkpf_load_component_to_wuobject_map(map_heap_id);

  } else if (mref == NATIVE_WKPF_METHOD_LOAD_LINK_DEFINITIONS) {
    heap_id_t links_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_error_code = wkpf_load_links(links_heap_id);    

  } else if (mref == NATIVE_WKPF_METHOD_GET_PORT_NUMBER_FOR_COMPONENT) {
    uint16_t component_id = (uint16_t)stack_pop_int();
    address_t node_id;
    uint8_t port_number;
    wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);
    stack_push(port_number);

  } else if (mref == NATIVE_WKPF_METHOD_IS_LOCAL_COMPONENT) {
    uint16_t component_id = (uint16_t)stack_pop_int();
    address_t node_id;
    uint8_t port_number;
    wkpf_error_code = wkpf_get_node_and_port_for_component(component_id, &node_id, &port_number);

    if (wkpf_error_code == WKPF_OK) {
      DEBUGF_WKPF("WKPF: Component %x is local\n", component_id);
      DEBUGF_WKPF("WKPF: found at (%x, %x)\n", node_id, port_number);
      stack_push(true);
    }
    else {
      DEBUGF_WKPF("WKPF: Component %x is not local\n", component_id);
      stack_push(false);
    }
  }
}
