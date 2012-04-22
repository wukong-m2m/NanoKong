#include "types.h"
#include "nvmcomm.h"
#include "wkpf.h"
#include "debug.h"

#define WKFPCOMM_SET_MESSAGE_HEADER_LEN 7

uint8_t message_buffer[NVMCOMM_MESSAGE_SIZE];
uint16_t next_sequence_number = 0;

void set_message_header(uint8_t port_number, uint8_t property_number, uint16_t profile_id, uint8_t datatype) {
  message_buffer[0] = (uint8_t)(next_sequence_number >> 8);
  message_buffer[1] = (uint8_t)(next_sequence_number++);
  message_buffer[2] = port_number;
  message_buffer[3] = (uint8_t)(profile_id >> 8);
  message_buffer[4] = (uint8_t)(profile_id);
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
  if(nvmcomm_send(dest_node_id, command, message_buffer, length) != 0)
    return WKPF_ERR_NVMCOMM_SEND_ERROR;
  nvmcomm_message *reply = nvmcomm_wait(100, (u08_t[]){command+1 /* the reply to this command */, NVMCOMM_WKPF_ERROR_R}, 2);
  if(reply == NULL)
    return WKPF_ERR_NVMCOMM_NO_REPLY;
  if(reply->command == NVMCOMM_WKPF_ERROR_R)
    return reply->payload[2]; // the WKPF error code sent by the other node.
  return WKPF_OK;
}

uint8_t send_set_property_int16(address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t profile_id, int16_t value) {
  set_message_header(port_number, property_number, profile_id, WKPF_PROPERTY_TYPE_INT16);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+0] = (uint8_t)(value >> 8);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+1] = (uint8_t)(value);
  return send_message(dest_node_id, NVMCOMM_WKPF_WRITE_PROPERTY, WKFPCOMM_SET_MESSAGE_HEADER_LEN+2);
}

uint8_t send_set_property_boolean(address_t dest_node_id, uint8_t port_number, uint8_t property_number, uint16_t profile_id, bool value) {
  set_message_header(port_number, property_number, profile_id, WKPF_PROPERTY_TYPE_BOOLEAN);
  message_buffer[WKFPCOMM_SET_MESSAGE_HEADER_LEN+0] = (uint8_t)(value);
  return send_message(dest_node_id, NVMCOMM_WKPF_WRITE_PROPERTY, WKFPCOMM_SET_MESSAGE_HEADER_LEN+1);
}

void wkpf_comm_handle_message(u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd) {
  uint8_t number_of_profiles;
  uint8_t number_of_endpoints;
  uint16_t profile_id;
  uint8_t port_number;
  uint8_t property_number;
  uint8_t retval;
  wkpf_local_endpoint *endpoint;
  
  switch (nvmcomm_command) {
    case NVMCOMM_WKPF_GET_PROFILE_LIST:
      number_of_profiles = wkpf_get_number_of_profiles();
      payload[2] = number_of_profiles;
      for (int i=0; i<number_of_profiles && i<14; i++) { // TODONR: i<14 is temporary to keep the length within MESSAGE_SIZE, but we should have a protocol that sends multiple messages
        wkpf_profile_definition *profile;
        wkpf_get_profile_by_index(i, &profile);
        payload[2*i + 3] = (uint8_t)(profile->profile_id >> 8);
        payload[2*i + 4] = (uint8_t)(profile->profile_id);
      }
      *response_size = 2*number_of_profiles + 3;//payload size 2*profiles + 2 bytes seqnr + 1 byte number of profiles
      *response_cmd = NVMCOMM_WKPF_GET_PROFILE_LIST_R;
    break;
    case NVMCOMM_WKPF_GET_ENDPOINT_LIST:
      number_of_endpoints = wkpf_get_number_of_endpoints();
      payload[2] = number_of_endpoints;
      for (int i=0; i<number_of_endpoints && i<9; i++) { // TODONR: i<9 is temporary to keep the length within MESSAGE_SIZE, but we should have a protocol that sends multiple messages
        wkpf_local_endpoint *endpoint;
        wkpf_get_endpoint_by_index(i, &endpoint);
        payload[3*i + 3] = (uint8_t)(endpoint->port_number);
        payload[3*i + 4] = (uint8_t)(endpoint->profile->profile_id >> 8);
        payload[3*i + 5] = (uint8_t)(endpoint->profile->profile_id);
      }
      *response_size = 3*number_of_endpoints + 3;//payload size 3*endpoints + 2 bytes seqnr + 1 byte number of profiles
      *response_cmd = NVMCOMM_WKPF_GET_ENDPOINT_LIST_R;
    break;
    case NVMCOMM_WKPF_READ_PROPERTY: // TODONR: check profileid
      port_number = payload[2];
      profile_id = (uint16_t)(payload[3]<<8)+(uint16_t)(payload[4]);
      property_number = payload[5];
      retval = wkpf_get_endpoint_by_port(port_number, &endpoint);
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
        break;
      }
      if (WKPF_GET_PROPERTY_DATATYPE(endpoint->profile->properties[property_number]) == WKPF_PROPERTY_TYPE_INT16) {
        int16_t value;
        retval = wkpf_external_read_property_int16(endpoint, property_number, &value);
        payload[6] = WKPF_GET_PROPERTY_DATATYPE(endpoint->profile->properties[property_number]);
        payload[7] = (uint8_t)(value>>8);
        payload[8] = (uint8_t)(value);
        *response_size = 9;//payload size
        *response_cmd = NVMCOMM_WKPF_READ_PROPERTY_R;        
      } else {
        bool value;
        retval = wkpf_external_read_property_boolean(endpoint, property_number, &value);
        payload[6] = WKPF_GET_PROPERTY_DATATYPE(endpoint->profile->properties[property_number]);
        payload[7] = (uint8_t)(value);
        *response_size = 8;//payload size
        *response_cmd = NVMCOMM_WKPF_READ_PROPERTY_R;                
      }
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
      }
    break;
    case NVMCOMM_WKPF_WRITE_PROPERTY:
      port_number = payload[2];
      profile_id = (uint16_t)(payload[3]<<8)+(uint16_t)(payload[4]);
      property_number = payload[5];
      retval = wkpf_get_endpoint_by_port(port_number, &endpoint);
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
        break;
      }
      for(int i=0;i<9;i++)
        DEBUGF_WKPFUPDATE("[%x] ", payload[i]);
      if (payload[6] == WKPF_PROPERTY_TYPE_INT16) {
        int16_t value;
        value = (int16_t)(payload[7]);
        value = (int16_t)(value<<8) + (int16_t)(payload[8]);
        retval = wkpf_external_write_property_int16(endpoint, property_number, value);
        *response_size = 6;//payload size
        *response_cmd = NVMCOMM_WKPF_WRITE_PROPERTY_R;        
      } else {
        bool value;
        value = (bool)(payload[7]);
        retval = wkpf_external_write_property_boolean(endpoint, property_number, value);
        *response_size = 6;//payload size
        *response_cmd = NVMCOMM_WKPF_WRITE_PROPERTY_R;                
      }
      if (retval != WKPF_OK) {
        payload [2] = retval;
        *response_cmd = NVMCOMM_WKPF_ERROR_R;
        *response_size = 3;//payload size
      }
    break;
  }
}
