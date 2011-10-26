#include "config.h"
#include "types.h"
#include "debug.h"
#include "vm.h"
#ifdef NVM_USE_COMMZWAVE
#include "nvmcomm_zwave.h"
#endif
#include "avr/avr_flash.h"

#include "nvmcomm3.h"

#ifdef NVMCOMM3

uint8_t nvc3_file_open = 0xFF;
uint16_t nvc3_file_pos = 0;
uint8_t nvc3_avr_flash_open = FALSE;

void handle_message(address_t src, u08_t *payload, u08_t length);
void nvmcomm_init(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_init();
  nvmcomm_zwave_setcallback(handle_message);
#endif
}

void nvmcomm_poll(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_poll();
#endif
}

int nvmcomm_send(address_t dest, u08_t nvc3_command, u08_t *payload, u08_t length) {
  if (length > NVC3_MESSAGE_SIZE)
    return -2; // Message too large
  return nvmcomm_zwave_send(dest, nvc3_command, payload, length, 0);
}
// Private

void handle_message(address_t src, u08_t *payload, u08_t length) {
  const u08_t nvmcomm3_command = payload[0];
  u08_t response_size = 0;
  u08_t response_cmd = 0;

#ifdef DEBUG
  DEBUGF_COMM("Handling message from "DBG8", length "DBG8":\n", src, length);
  for (size8_t i=0; i<length; ++i) {
    DEBUGF_COMM(" "DBG8"", payload[i]);
  }
  DEBUGF_COMM("\n");
#endif
  
  switch (nvmcomm3_command) {
    case NVC3_CMD_FOPEN:
      if (payload[1] <= NVC3_MAX_FID) {
        DEBUGF_COMM("Open file "DBG8"\n", payload[1]);
        nvc3_file_open = payload[1];
        nvc3_file_pos = 0;
      }
    break;
    case NVC3_CMD_FCLOSE:
      DEBUGF_COMM("Closing file\n");
			if (nvc3_avr_flash_open == TRUE)
				avr_flash_close();
      nvc3_file_open = 0xFF;
    break;
    case NVC3_CMD_FSEEK:
      nvc3_file_pos = ((u16_t)payload[1]<<8) + payload[2];
      DEBUGF_COMM("Seek to position "DBG16"\n", nvc3_file_pos);
    break;
    case NVC3_CMD_RDFILE:
      if (nvc3_file_open == NVC3_FILE_FIRMWARE) {
        u08_t *addr = nvmfile_get_base();
        addr += nvc3_file_pos;

        response_size = payload[1];
        if (response_size < 39) { // TODO: check for buffer size (depends on protocol)
          for (size8_t i=0; i<response_size; ++i) {
            payload[i] = nvmfile_read08(addr++);
            ++nvc3_file_pos;
          }
        }
        response_cmd = NVC3_CMD_RDFILE_R;
      }
    break;
    case NVC3_CMD_WRFILE:
      if (nvc3_file_open == NVC3_FILE_FIRMWARE) {
	 			if (nvc3_avr_flash_open == FALSE) {
					nvc3_avr_flash_open = TRUE;
					avr_flash_open(0x4000); // TODO: ugly hack
        	vm_set_runlevel(NVM_RUNLVL_CONF); // opening firmware for writing implies conf runlevel
				}
        DEBUGF_COMM("Write "DBG8" bytes at position "DBG16", address "DBG16".\n", length-1, nvc3_file_pos, nvc3_file_pos);
				avr_flash_write(length-1, payload+1);
        nvc3_file_pos += length-1;
      }
    break;
    case NVC3_CMD_GETRUNLVL: 
      payload[0] = nvm_runlevel;
      response_cmd = NVC3_CMD_GETRUNLVL_R;
      response_size = 1;
    break;
    case NVC3_CMD_SETRUNLVL:
      DEBUGF_COMM("Goto runlevel "DBG8"\n", payload[1]);
      vm_set_runlevel(payload[1]);
      response_cmd = NVC3_CMD_SETRUNLVL_R;
    break;
  }
  if (response_cmd > 0) {
    nvmcomm_send(src, response_cmd, payload, response_size);
  }
}

#endif // NVMCOMM3
