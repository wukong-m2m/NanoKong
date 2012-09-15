#include "config.h"
#include "types.h"
#include "debug.h"
#include "vm.h"
#include "avr/avr_flash.h"
#include "nvmcomm.h"
#include "delay.h"
#include "nvmfile.h"
#include "wkpf_comm.h"

#ifdef NVM_USE_COMM

uint8_t nvc3_avr_reprogramming = FALSE;
uint16_t nvc3_avr_reprogramming_pos;

uint8_t nvc3_appmsg_buf[NVMCOMM_MESSAGE_SIZE];
uint8_t nvc3_appmsg_size = 0; // 0 if the buffer is not in use (so we can receive a message), otherwise indicates the length of the received message.
uint8_t nvc3_appmsg_reply = 0;

// When handle_message finds any of these commands it will store the message in nvmcomm_wait_received_messages so nvmcomm_wait can return it.
uint8_t *nvmcomm_wait_commands;
uint8_t nvmcomm_wait_number_of_commands;
nvmcomm_message nvmcomm_wait_received_message;

void handle_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t length);

void nvmcomm_init(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_init();
  nvmcomm_zwave_setcallback(handle_message);
#endif
#ifdef NVM_USE_COMMXBEE
  nvmcomm_xbee_init();
  nvmcomm_xbee_setcallback(handle_message);
#endif
}

void nvmcomm_poll(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_poll();
#endif
#ifdef NVM_USE_COMMXBEE
  nvmcomm_xbee_poll();
#endif
}

int nvmcomm_send(address_t dest, u08_t nvc3_command, u08_t *payload, u08_t length) {
  if (length > NVMCOMM_MESSAGE_SIZE) {
    DEBUGF_COMM("message oversized\n");
    return -2; // Message too large
  }
  int retval = -1;
  DEBUGF_COMM("nvmcomm_send\n");
#ifdef NVM_USE_COMMZWAVE
  retval = nvmcomm_zwave_send(dest, nvc3_command, payload, length, TRANSMIT_OPTION_ACK + TRANSMIT_OPTION_AUTO_ROUTE);
  if (retval == 0) {
    if (nvc3_command==NVMCOMM_CMD_APPMSG) {
      nvc3_appmsg_reply = NVMCOMM_APPMSG_WAIT_ACK;
      return retval;
    }
  }
#endif
#ifdef NVM_USE_COMMXBEE
  retval = nvmcomm_xbee_send(dest, nvc3_command, payload, length, 0);
  if (retval == 0) {
    if (nvc3_command==NVMCOMM_CMD_APPMSG) {
      nvc3_appmsg_reply = NVMCOMM_APPMSG_WAIT_ACK;
      return retval;
    }
  }
#endif
  return retval;
}
// Private

void handle_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t length) {
  u08_t response_size = 0;
  u08_t response_cmd = 0;
  uint16_t pos_in_message;

#ifdef DEBUG
  DEBUGF_COMM("Handling command "DBG8" from "DBG8", length "DBG8":\n", nvmcomm_command, src, length);
  for (size8_t i=0; i<length; ++i) {
    DEBUGF_COMM(" "DBG8"", payload[i]);
  }
  DEBUGF_COMM("\n");
#endif

  if (nvmcomm_wait_number_of_commands > 0) {
    // nvmcomm_wait is waiting for a particular type of message. probably a response to a message sent earlier.
    // if this message is of that type, store it in nvmcomm_wait_received_message so nvmcomm_wait can return it.
    // if not, handle it as a normal message
    for (int i=0; i<nvmcomm_wait_number_of_commands; i++) {
      if (nvmcomm_command == nvmcomm_wait_commands[i]) {
        nvmcomm_wait_received_message.command = nvmcomm_command;
        nvmcomm_wait_received_message.payload = payload;
        nvmcomm_wait_received_message.payload_length = length;
      }
    }
  }
  
  switch (nvmcomm_command) {
    case NVMCOMM_CMD_REPRG_OPEN:
      DEBUGF_COMM("Initialise reprogramming.\n");
      nvc3_avr_reprogramming = TRUE;
      nvc3_avr_reprogramming_pos = 0;
      avr_flash_open(bytecode_address);
      DEBUGF_COMM("Going to runlevel NVM_RUNLVL_CONF.\n");
      vm_set_runlevel(NVM_RUNLVL_CONF);
      response_cmd = NVMCOMM_CMD_REPRG_OPEN_R;
      payload[2] = (uint8_t)(AVR_FLASH_PAGESIZE>>8);
      payload[3] = (uint8_t)(AVR_FLASH_PAGESIZE);
      response_size = 4;
    break;
    case NVMCOMM_CMD_REPRG_WRITE:
      pos_in_message = (((uint16_t)payload[2])<<8) + ((uint16_t)payload[3]);
      uint16_t expected_pos = nvc3_avr_reprogramming_pos;
      DEBUGF_COMM("Received program packet for address "DBG16", current position: "DBG16".\n", pos_in_message, nvc3_avr_reprogramming_pos);
      u08_t codelength = length - 4;
      u08_t *codepayload = payload + 4;
      if (pos_in_message == expected_pos) {
        DEBUGF_COMM("Write "DBG8" bytes at position "DBG16".\n", codelength, nvc3_avr_reprogramming_pos);
        avr_flash_write(codelength, codepayload);
        nvc3_avr_reprogramming_pos += codelength;
      }
      if (pos_in_message/(uint16_t)AVR_FLASH_PAGESIZE != (pos_in_message+(uint16_t)codelength)/(uint16_t)AVR_FLASH_PAGESIZE) {
        // Crossing page boundary, send a NVMCOMM_CMD_REPRG_WRITE_R_OK or NVMCOMM_CMD_REPRG_WRITE_R_RETRANSMIT
        if (pos_in_message == expected_pos) {
          DEBUGF_COMM("Page boundary reached. Sending REPRG_WRITE_R_OK.\n");
          response_cmd = NVMCOMM_CMD_REPRG_WRITE_R_OK;
          response_size = 2; // now the sequence numbers are needed
        } else {
          DEBUGF_COMM("Page boundary reached, positions don't match. Sending WRITE_RETRANSMIT request.\n");
          response_cmd = NVMCOMM_CMD_REPRG_WRITE_R_RETRANSMIT;
          payload[2] = (uint8_t)(nvc3_avr_reprogramming_pos>>8);
          payload[3] = (uint8_t)(nvc3_avr_reprogramming_pos);
          response_size = 4;
        }
      }
    break;
    case NVMCOMM_CMD_REPRG_COMMIT:
      pos_in_message = (((uint16_t)payload[2])<<8) + ((uint16_t)payload[3]);
      DEBUGF_COMM("Received commit request for code up to address "DBG16", current position: "DBG16".\n", pos_in_message, nvc3_avr_reprogramming_pos);
      if (pos_in_message != nvc3_avr_reprogramming_pos) {
        DEBUGF_COMM("Positions don't match. Sending COMMIT_RETRANSMIT request.");
        response_cmd = NVMCOMM_CMD_REPRG_COMMIT_R_RETRANSMIT;
        payload[2] = (uint8_t)(nvc3_avr_reprogramming_pos>>8);
        payload[3] = (uint8_t)(nvc3_avr_reprogramming_pos);
        response_size = 4;
      } else if (0==1) {
        // TODO: add checksum, send NVMCOMM_CMD_REPRG_COMMIT_R_FAILED if they don't match.
        response_cmd = NVMCOMM_CMD_REPRG_COMMIT_R_FAILED;
        response_size = 2; // now the sequence numbers are needed
      } else {
        DEBUGF_COMM("Committing new code.\n");
        DEBUGF_COMM("Flushing pending writes to flash.\n");
        avr_flash_close();
        nvc3_avr_reprogramming = FALSE;
        response_cmd = NVMCOMM_CMD_REPRG_COMMIT_R_OK;
        response_size = 2; // now the sequence numbers are needed
      }
    break;
    case NVMCOMM_CMD_GETRUNLVL: 
      payload[2] = nvm_runlevel;
      response_size = 3;
      response_cmd = NVMCOMM_CMD_GETRUNLVL_R;
    break;
    case NVMCOMM_CMD_SETRUNLVL:
      DEBUGF_COMM("Goto runlevel "DBG8"\n", payload[2]);
      nvmcomm_send(src, NVMCOMM_CMD_SETRUNLVL_R, payload, 2); // Send here instead of at the bottom because we might be resetting. sequence numbers are needed
      response_cmd = 0;
      response_size = 2; // sequence numbers are needed
      vm_set_runlevel(payload[2]);
    break;
    case NVMCOMM_CMD_APPMSG:
      if (nvc3_appmsg_size == 0) {
        for (size8_t i=2; i<length; ++i) {
          nvc3_appmsg_buf[i] = payload[i];
        }
        nvc3_appmsg_size = length;
        payload[2] = NVMCOMM_APPMSG_ACK;
        DEBUGF_COMM("Received some data intended for Java: ACK\n");
      } else {
        payload[2] = NVMCOMM_APPMSG_BUSY;
        DEBUGF_COMM("Received some data intended for Java: BUSY!\n");
      }
      response_size = 3;
      response_cmd = NVMCOMM_CMD_APPMSG_R;
    break;
    case NVMCOMM_CMD_APPMSG_R:
      // TODO: expose this to Java. Make ACKs optional.
      nvc3_appmsg_reply = payload[2];
    break;
    case NVMCOMM_WKPF_GET_LOCATION:
    case NVMCOMM_WKPF_SET_LOCATION:
    case NVMCOMM_WKPF_GET_WUCLASS_LIST:
    case NVMCOMM_WKPF_GET_WUOBJECT_LIST:
    case NVMCOMM_WKPF_READ_PROPERTY:
    case NVMCOMM_WKPF_WRITE_PROPERTY:
    case NVMCOMM_WKPF_REQUEST_PROPERTY_INIT:
      wkpf_comm_handle_message(nvmcomm_command, payload, &response_size, &response_cmd);
    break;
  }
  if (response_cmd > 0) {
#ifdef DEBUG
    DEBUGF_COMM("response_size: "DBG8"\n", response_size);
#endif
    nvmcomm_send(src, response_cmd, payload, response_size);
  }
}

nvmcomm_message *nvmcomm_wait(u16_t wait_msec, u08_t *commands, u08_t number_of_commands) {
  nvmcomm_wait_commands = commands;
  nvmcomm_wait_number_of_commands = number_of_commands;
  nvmcomm_wait_received_message.command = 0;
  
  while(wait_msec > 0) {
    if (wait_msec > 10) {
      delay(MILLISEC(10));
      wait_msec -= 10;
    } else {
      delay(MILLISEC(wait_msec));
      wait_msec = 0;
    }
    nvmcomm_poll();
    if (nvmcomm_wait_received_message.command != 0)
      nvmcomm_wait_number_of_commands = 0;
      return &nvmcomm_wait_received_message;
  }
  nvmcomm_wait_number_of_commands = 0;
  return NULL;
}

address_t nvmcomm_get_node_id() {
#ifdef NVM_USE_COMMZWAVE
  return nvmcomm_zwave_get_node_id();
#else
  return 0;
#endif // NVM_USE_COMMZWAVE
}
#endif // NVM_USE_COMM
