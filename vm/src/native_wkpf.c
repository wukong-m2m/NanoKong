#include "native.h"
#include "stack.h"
#include "debug.h"
#include "nvmcomm.h"
#include "heap.h"
#include "array.h"
#include "wkpf.h"

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
    DEBUGF_WKPF("WKPF: setPropertyBoolean (local). Component %x, property %x, value %x\n", component_id, property_number, value);
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

  } else if (mref == NATIVE_WKPF_METHOD_SELECT) {
    wkpf_local_wuobject *wuobject;
//TODONR: TMP    while(true) {
      // Process incoming messages
      nvmcomm_poll();
      // Propagate any dirty properties
      if (wkpf_any_property_dirty()) { // this call is still here to have a place to mark failed properties as dirty again. should probably be refactored
        uint8_t port_number;
        uint8_t property_number;
        int16_t value;
        while(wkpf_get_next_dirty_property(&port_number, &property_number, &value)) {
          nvmcomm_poll(); // Process incoming messages
          wkpf_error_code = wkpf_propagate_property(port_number, property_number, value);
          if (wkpf_error_code != WKPF_OK) { // TODONR: need better retry mechanism
            DEBUGF_WKPF("WKPF: ------!!!------ Propagating property failed: port %x property %x error %x\n", port_number, property_number, wkpf_error_code);
            wkpf_propagating_dirty_property_failed(port_number, property_number);
          }
        }
      }
      // Check if any wuobjects need updates
      if (wkpf_get_next_wuobject_to_update(&wuobject)) {
        stack_push(wuobject->virtual_wuclass_instance_heap_id | NVM_TYPE_MASK);
        DEBUGF_WKPF("WKPF: WKPF.select returning wuclass at port %x.\n", wuobject->port_number);
        return;
      }
      // TODONR: Temporarily return null anyway to allow Java to trigger updates while don't have a scheduling mechanism yet.
      // In the final version select() should just wait until either there's a dirty property, or a wuclass needs to be updated.
      DEBUGF_WKPF("WKPF: WKPF.select temporarily returning null until we can schedule update() properly.\n");
      stack_push(0);
//TODONR: TMP    }

  } else if (mref == NATIVE_WKPF_METHOD_GETMYNODEID) {
    stack_push(nvmcomm_get_node_id());

  } else if (mref == NATIVE_WKPF_METHOD_LOAD_COMPONENT_MAP) {
    heap_id_t map_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_error_code = wkpf_load_component_to_wuobject_map(map_heap_id);

  } else if (mref == NATIVE_WKPF_METHOD_LOAD_LINK_DEFINITIONS) {
    heap_id_t links_heap_id = stack_pop() & ~NVM_TYPE_MASK;
    wkpf_error_code = wkpf_load_links(links_heap_id);    
  }
}
