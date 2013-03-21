#include <string.h>
#include "types.h"
#include "nvmcomm.h"
#include "wkpf.h"
#include "debug.h"
#include "delay.h"
#include "vm.h"
#include "wkpf_config.h"

#define WKFPCOMM_SET_MESSAGE_HEADER_LEN 7

uint8_t message_buffer[NVMCOMM_MESSAGE_SIZE];

void set_message_header(uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, uint8_t datatype) {
  set_message_sequence_number(message_buffer, NULL);
  message_buffer[2] = port_number;
  message_buffer[3] = (uint8_t)(wuclass_id >> 8);
  message_buffer[4] = (uint8_t)(wuclass_id);
  message_buffer[5] = property_number;
  message_buffer[6] = datatype;
}

uint8_t send_message(address_t dest_node_id, uint8_t command, uint8_t length) {
  #ifdef DEBUG
  DEBUGF_WKPF("WKPF: sending property set command to %x:", dest_node_id);
  for(int i=0; i<length; i++) {
    DEBUGF_WKPF("[%x] ", message_buffer[i]);
  }
  DEBUGF_WKPF("\n");
  #endif
  // Send
  if(nvmcomm_send(dest_node_id, command, message_buffer, length) != 0)
    return WKPF_ERR_NVMCOMM_SEND_ERROR;
  // Wait for a reply
  uint32_t timeout = nvm_current_time + 100;
  while(nvm_current_time < timeout) {
    nvmcomm_message *reply = nvmcomm_wait(100, (u08_t[]){command+1 /* the reply to this command */, NVMCOMM_WKPF_ERROR_R}, 2);
    if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
          && check_sequence_number(reply->payload, message_buffer)) {
      // This message a reply to our last sent message
      if(reply->command != NVMCOMM_WKPF_ERROR_R)
        return WKPF_OK;
      else
        return reply->payload[2]; // the WKPF error code sent by the other node.
    }
  }
  return WKPF_ERR_NVMCOMM_NO_REPLY; // Give up
}

uint8_t wkpf_send_set_property_int16(address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, int16_t value) {
  set_message_header(port_number, property_number, wuclass_id, WKPF_PROPERTY_TYPE_SHORT);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+0] = (uint8_t)(value >> 8);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+1] = (uint8_t)(value);
  return send_message(dest_node_id, NVMCOMM_WKPF_WRITE_PROPERTY, WKFPCOMM_SET_MESSAGE_HEADER_LEN+2);
}

uint8_t wkpf_send_set_property_boolean(address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, bool value) {
  set_message_header(port_number, property_number, wuclass_id, WKPF_PROPERTY_TYPE_BOOLEAN);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+0] = (uint8_t)(value);
  return send_message(dest_node_id, NVMCOMM_WKPF_WRITE_PROPERTY, WKFPCOMM_SET_MESSAGE_HEADER_LEN+1);
}

uint8_t wkpf_send_set_property_refresh_rate(address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t wuclass_id, wkpf_refresh_rate_t value) {
  set_message_header(port_number, property_number, wuclass_id, WKPF_PROPERTY_TYPE_REFRESH_RATE);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+0] = (uint8_t)(value >> 8);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+1] = (uint8_t)(value);
  return send_message(dest_node_id, NVMCOMM_WKPF_WRITE_PROPERTY, WKFPCOMM_SET_MESSAGE_HEADER_LEN+2);
}

uint8_t wkpf_send_request_property_init(address_t dest_node_id, uint8_t port_number, uint8_t property_number) {
  set_message_header(port_number, property_number, 0, 0); // 0 because this message doesn't take a data type or wuclass ID
  return send_message(dest_node_id, NVMCOMM_WKPF_REQUEST_PROPERTY_INIT, 6);
}


void wkpf_comm_handle_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd) {
  uint8_t number_of_wuclasses;
  uint8_t number_of_wuobjects;
  // TODONR: uint16_t wuclass_id;
  uint8_t port_number;
  uint8_t property_number;
  uint8_t retval;
  wkpf_local_wuobject *wuobject;

  if (nvm_runlevel != NVM_RUNLVL_VM)
    return;

  switch (nvmcomm_command) {
    case NVMCOMM_WKPF_GET_LOCATION:
      {
        // Format of get_location request messages: payload[2] offset of the first byte requested
        // Format of get_location return messages: payload[2..] the part of the location string

        // The length of the location is stored by the master as the first byte of the string.

        // Get the offset of the requested data within the location string
        uint8_t requested_offset = payload[2];

        // Read the EEPROM
        char* location_in_message_payload = (char*)payload+2;
//        uint8_t length = wkpf_config_get_part_of_location_string(location_in_message_payload, requested_offset, NVMCOMM_MESSAGE_SIZE-2);
        uint8_t length = wkpf_config_get_part_of_location_string(location_in_message_payload, requested_offset, 5);

        DEBUGF_WKPF("NVMCOMM_WKPF_GET_LOCATION: Reading %d bytes at offset %d\n", length, requested_offset);

        *response_cmd = NVMCOMM_WKPF_GET_LOCATION_R;
        *response_size = 2 + length;
      }
    break;
    case NVMCOMM_WKPF_SET_LOCATION:
      {
        // Format of set_location request messages: payload[2] offset of part of the location string being sent
        // Format of set_location request messages: payload[3] the length of part of the location string being sent
        // Format of set_location request messages: payload[4..] the part of the location string
        // Format of set_location return messages: payload[2] the wkpf return code

        uint8_t written_offset = payload[2];
        uint8_t length = payload[3];

        DEBUGF_WKPF("NVMCOMM_WKPF_SET_LOCATION: Writing %d bytes at offset %d\n", length, written_offset);

        // Read the EEPROM
        retval = wkpf_config_set_part_of_location_string((char*) payload+4, written_offset, length);

        // Send response
        if (retval == WKPF_OK) {
          *response_cmd = NVMCOMM_WKPF_SET_LOCATION_R;
        } else {
          *response_cmd = NVMCOMM_WKPF_ERROR_R;
        }
        payload[2] = retval;       
        *response_size = 3;//payload size
      }
    break;
    case NVMCOMM_WKPF_GET_FEATURES:
      {
        int count = 0;
        for (int i=0; i<=WKPF_MAX_FEATURE_NUMBER; i++) { // Needs to be changed if we have more features than fits in a single message, but for now it will work fine.
          if (wkpf_config_get_feature_enabled(i)) {
            payload[3+count++] = i;
          }
        }
        payload[2] = count;
        *response_cmd = NVMCOMM_WKPF_GET_FEATURES_R;
        *response_size = 3+count;//payload size
      }
    break;
    case NVMCOMM_WKPF_SET_FEATURE:
      retval = wkpf_config_set_feature_enabled(payload[2], payload[3]);
      if (retval == WKPF_OK) {
          *response_cmd = NVMCOMM_WKPF_SET_FEATURE_R;
          *response_size = 2;//payload size
        } else {
          payload[2] = retval;       
          *response_cmd = NVMCOMM_WKPF_ERROR_R;
          *response_size = 3;//payload size
        }
    break;
    case NVMCOMM_WKPF_GET_WUCLASS_LIST:
      number_of_wuclasses = wkpf_get_number_of_wuclasses();
      payload[2] = number_of_wuclasses;
      for (int i=0; i<number_of_wuclasses && i<9; i++) { // TODONR: i<9 is temporary to keep the length within MESSAGE_SIZE, but we should have a protocol that sends multiple messages
        wkpf_wuclass_definition *wuclass;
        wkpf_get_wuclass_by_index(i, &wuclass);
        payload[3*i + 3] = (uint8_t)(wuclass->wuclass_id >> 8);
        payload[3*i + 4] = (uint8_t)(wuclass->wuclass_id);
        payload[3*i + 5] = WKPF_IS_VIRTUAL_WUCLASS(wuclass) ? 1 : 0;
      }
      *response_size = 3*number_of_wuclasses + 3;//payload size 2*wuclasses + 2 bytes seqnr + 1 byte number of wuclasses
      *response_cmd = NVMCOMM_WKPF_GET_WUCLASS_LIST_R;
    break;
    case NVMCOMM_WKPF_GET_WUOBJECT_LIST:
      number_of_wuobjects = wkpf_get_number_of_wuobjects();
      payload[2] = number_of_wuobjects;
      for (int i=0; i<number_of_wuobjects && i<9; i++) { // TODONR: i<9 is temporary to keep the length within MESSAGE_SIZE, but we should have a protocol that sends multiple messages
        wkpf_local_wuobject *wuobject;
        wkpf_get_wuobject_by_index(i, &wuobject);
        payload[3*i + 3] = (uint8_t)(wuobject->port_number);
        payload[3*i + 4] = (uint8_t)(wuobject->wuclass->wuclass_id >> 8);
        payload[3*i + 5] = (uint8_t)(wuobject->wuclass->wuclass_id);
      }
      *response_size = 3*number_of_wuobjects + 3;//payload size 3*wuobjects + 2 bytes seqnr + 1 byte number of wuclasses
      *response_cmd = NVMCOMM_WKPF_GET_WUOBJECT_LIST_R;
    break;
    case NVMCOMM_WKPF_READ_PROPERTY: // TODONR: check wuclassid
      port_number = payload[2];
      // TODONR: wuclass_id = (uint16_t)(payload[3]<<8)+(uint16_t)(payload[4]);
      property_number = payload[5];
      retval = wkpf_get_wuobject_by_port(port_number, &wuobject);
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
        break;
      }
      uint8_t property_status;
      wkpf_get_property_status(wuobject, property_number, &property_status);
      if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_SHORT) {
        int16_t value;
        retval = wkpf_external_read_property_int16(wuobject, property_number, &value);
        payload[6] = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
        payload[7] = property_status;
        payload[8] = (uint8_t)(value>>8);
        payload[9] = (uint8_t)(value);
        *response_size = 10;//payload size
        *response_cmd = NVMCOMM_WKPF_READ_PROPERTY_R;        
      } else if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_BOOLEAN) {
        bool value;
        retval = wkpf_external_read_property_boolean(wuobject, property_number, &value);
        payload[6] = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
        payload[7] = property_status;
        payload[8] = (uint8_t)(value);
        *response_size = 9;//payload size
        *response_cmd = NVMCOMM_WKPF_READ_PROPERTY_R;                
      } else if (WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]) == WKPF_PROPERTY_TYPE_REFRESH_RATE) {
        wkpf_refresh_rate_t value;
        retval = wkpf_external_read_property_refresh_rate(wuobject, property_number, &value);
        payload[6] = WKPF_GET_PROPERTY_DATATYPE(wuobject->wuclass->properties[property_number]);
        payload[7] = property_status;
        payload[8] = (uint8_t)(value>>8);
        payload[9] = (uint8_t)(value);
        *response_size = 10;//payload size
        *response_cmd = NVMCOMM_WKPF_READ_PROPERTY_R;        
      } else
        retval = WKPF_ERR_SHOULDNT_HAPPEN;
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
      }
    break;
    case NVMCOMM_WKPF_WRITE_PROPERTY:
      port_number = payload[2];
      // TODONR: wuclass_id = (uint16_t)(payload[3]<<8)+(uint16_t)(payload[4]);
      property_number = payload[5];
      uint16_t wuclass_id;
      link_entry link;
      wuclass_id = (int16_t)(payload[3]);
      wuclass_id = (int16_t)(wuclass_id<<8) + (int16_t)(payload[4]);
      wkpf_get_link_by_dest_property_and_dest_wuclass_id(property_number, wuclass_id, &link);

      // TODO: should we do that now?
      // If the sender is not a leader
      if (!wkpf_node_is_leader(link.src_component_id, src)) {
        /**response_cmd = NVMCOMM_WKPF_ERROR_R;*/
        /**response_size = 3;//payload size*/
        /*break;*/
      }

      retval = wkpf_get_wuobject_by_port(port_number, &wuobject);
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
        break;
      }
      if (payload[6] == WKPF_PROPERTY_TYPE_SHORT) {
        int16_t value;
        value = (int16_t)(payload[7]);
        value = (int16_t)(value<<8) + (int16_t)(payload[8]);
        retval = wkpf_external_write_property_int16(wuobject, property_number, value);
        *response_size = 6;//payload size
        *response_cmd = NVMCOMM_WKPF_WRITE_PROPERTY_R;        
      } else if (payload[6] == WKPF_PROPERTY_TYPE_BOOLEAN) {
        bool value;
        value = (bool)(payload[7]);
        retval = wkpf_external_write_property_boolean(wuobject, property_number, value);
        *response_size = 6;//payload size
        *response_cmd = NVMCOMM_WKPF_WRITE_PROPERTY_R;                
      } else if (payload[6] == WKPF_PROPERTY_TYPE_REFRESH_RATE) {
        int16_t value;
        value = (int16_t)(payload[7]);
        value = (int16_t)(value<<8) + (int16_t)(payload[8]);
        retval = wkpf_external_write_property_refresh_rate(wuobject, property_number, value);
        *response_size = 6;//payload size
        *response_cmd = NVMCOMM_WKPF_WRITE_PROPERTY_R;
      } else
        retval = WKPF_ERR_SHOULDNT_HAPPEN;
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
      }
    break;
    case NVMCOMM_WKPF_REQUEST_PROPERTY_INIT:
      port_number = payload[2];
      // TODONR: wuclass_id = (uint16_t)(payload[3]<<8)+(uint16_t)(payload[4]);
      property_number = payload[5];
      retval = wkpf_get_wuobject_by_port(port_number, &wuobject);
      if (retval == WKPF_OK) {
        retval = wkpf_property_needs_initialisation_push(wuobject, property_number);
      }
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
      } else {
        *response_size = 6;//payload size
        *response_cmd = NVMCOMM_WKPF_REQUEST_PROPERTY_INIT_R;                
      }
    break;
  }
}
