#include "config.h"
#include "types.h"
#include "debug.h"
#include "vm.h"
#ifdef NVM_USE_COMMZWAVE
#include "nvmcomm_zwave.h"
#endif
#ifdef NVM_USE_COMMXBEE
#include "nvmcomm_xbee.h"
#endif
#include "avr/avr_flash.h"
#include "nvmcomm.h"

#ifdef NVM_USE_COMM

uint8_t testretransmission = 0;

uint8_t nvc3_avr_reprogramming = FALSE;
uint16_t nvc3_avr_reprogramming_pos;

uint8_t nvc3_appmsg_buf[NVMCOMM_MESSAGE_SIZE];
uint8_t nvc3_appmsg_size = 0; // 0 if the buffer is not in use (so we can receive a message), otherwise indicates the length of the received message.
uint8_t nvc3_appmsg_reply = 0;

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
  if (length > NVMCOMM_MESSAGE_SIZE)
    return -2; // Message too large
  int retval = -1, retval2 = -1;
  DEBUGF_COMM("nvmcomm_send\n");
#ifdef NVM_USE_COMMZWAVE
  retval = nvmcomm_zwave_send(dest, nvc3_command, payload, length, TRANSMIT_OPTION_ACK + TRANSMIT_OPTION_AUTO_ROUTE);
#endif
#ifdef NVM_USE_COMMXBEE
  retval2 = nvmcomm_xbee_send(dest, nvc3_command, payload, length, 0);
#endif

  if ((retval2 == 0 || retval == 0) && nvc3_command==NVMCOMM_CMD_APPMSG)
    nvc3_appmsg_reply = NVMCOMM_APPMSG_WAIT_ACK;
  return retval2;
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
  
  switch (nvmcomm_command) {
    case NVMCOMM_CMD_REPRG_OPEN:
      DEBUGF_COMM("Initialise reprogramming.\n");
      nvc3_avr_reprogramming = TRUE;
      nvc3_avr_reprogramming_pos = 0;
      avr_flash_open(0x8000); // TODO: ugly hack
      DEBUGF_COMM("Going to runlevel NVM_RUNLVL_CONF.\n");
      vm_set_runlevel(NVM_RUNLVL_CONF);
      response_cmd = NVMCOMM_CMD_REPRG_OPEN_R;
      payload[0] = (uint8_t)(AVR_FLASH_PAGESIZE>>8);
      payload[1] = (uint8_t)(AVR_FLASH_PAGESIZE);
      response_size = 2;
    break;
    case NVMCOMM_CMD_REPRG_WRITE:
      pos_in_message = (((uint16_t)payload[0])<<8) + ((uint16_t)payload[1]);
      uint16_t expected_pos = nvc3_avr_reprogramming_pos;
      DEBUGF_COMM("Received program packet for address "DBG16", current position: "DBG16".\n", pos_in_message, nvc3_avr_reprogramming_pos);
      u08_t codelength = length - 2;
      u08_t *codepayload = payload + 2;
      if (pos_in_message == expected_pos) {
        DEBUGF_COMM("Write "DBG8" bytes at position "DBG16".\n", codelength, nvc3_avr_reprogramming_pos);
				avr_flash_write(codelength, codepayload);
        nvc3_avr_reprogramming_pos += codelength;
      }
      if (pos_in_message/(uint16_t)AVR_FLASH_PAGESIZE != (pos_in_message+(uint16_t)codelength)/(uint16_t)AVR_FLASH_PAGESIZE) {
        // Crossing page boundary, send a NVMCOMM_CMD_REPRG_WRITE_R_OK or NVMCOMM_CMD_REPRG_WRITE_R_RETRANSMIT
        if (pos_in_message == expected_pos) {
          DEBUGF_COMM("Page boundary reached. Sending REPRG_WRITE_R_OK.");
          response_cmd = NVMCOMM_CMD_REPRG_WRITE_R_OK;
        } else {
          DEBUGF_COMM("Page boundary reached, positions don't match. Sending WRITE_RETRANSMIT request.");
          response_cmd = NVMCOMM_CMD_REPRG_WRITE_R_RETRANSMIT;
          payload[0] = (uint8_t)(nvc3_avr_reprogramming_pos>>8);
          payload[1] = (uint8_t)(nvc3_avr_reprogramming_pos);
          response_size = 2;
        }
      }
    break;
    case NVMCOMM_CMD_REPRG_COMMIT:
      pos_in_message = (((uint16_t)payload[0])<<8) + ((uint16_t)payload[1]);
      DEBUGF_COMM("Received commit request for code up to address "DBG16", current position: "DBG16".\n", pos_in_message, nvc3_avr_reprogramming_pos);
      if (pos_in_message != nvc3_avr_reprogramming_pos) {
        DEBUGF_COMM("Positions don't match. Sending COMMIT_RETRANSMIT request.");
        response_cmd = NVMCOMM_CMD_REPRG_COMMIT_R_RETRANSMIT;
        payload[0] = (uint8_t)(nvc3_avr_reprogramming_pos>>8);
        payload[1] = (uint8_t)(nvc3_avr_reprogramming_pos);
        response_size = 2;
      } else if (0==1) {
        // TODO: add checksum, send NVMCOMM_CMD_REPRG_COMMIT_R_FAILED if they don't match.
        response_cmd = NVMCOMM_CMD_REPRG_COMMIT_R_FAILED;
      } else {
        DEBUGF_COMM("Committing new code.\n");
        DEBUGF_COMM("Flushing pending writes to flash.\n");
        avr_flash_close();
        nvc3_avr_reprogramming = FALSE;
        response_cmd = NVMCOMM_CMD_REPRG_COMMIT_R_OK;
      }
    break;
    case NVMCOMM_CMD_GETRUNLVL: 
      payload[0] = nvm_runlevel;
      response_size = 1;
      response_cmd = NVMCOMM_CMD_GETRUNLVL_R;
    break;
    case NVMCOMM_CMD_SETRUNLVL:
      DEBUGF_COMM("Goto runlevel "DBG8"\n", payload[0]);
      vm_set_runlevel(payload[0]);
      response_cmd = NVMCOMM_CMD_SETRUNLVL_R;
    break;
    case NVMCOMM_CMD_APPMSG:
      if (nvc3_appmsg_size == 0) {
        for (size8_t i=0; i<length; ++i) {
          nvc3_appmsg_buf[i] = payload[i];
        }
        nvc3_appmsg_size = length;
        payload[0] = NVMCOMM_APPMSG_ACK;
        DEBUGF_COMM("Received some data intended for Java: ACK\n");
      } else {
        payload[0] = NVMCOMM_APPMSG_BUSY;
        DEBUGF_COMM("Received some data intended for Java: BUSY!\n");
      }
      response_size = 1;
      response_cmd = NVMCOMM_CMD_APPMSG_R;
    break;
    case NVMCOMM_CMD_APPMSG_R:
      // TODO: expose this to Java. Make ACKs optional.
      nvc3_appmsg_reply = payload[0];
    break;
  }
  if (response_cmd > 0) {
    nvmcomm_send(src, response_cmd, payload, response_size);
  }
}

#endif // NVM_USE_COMM
