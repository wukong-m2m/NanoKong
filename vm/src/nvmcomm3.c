#include "config.h"
#include "types.h"
#include "debug.h"
#include "vm.h"
#ifdef NVM_USE_COMMZWAVE
#include "nvmcomm_zwave.h"
#endif

#include "nvmcomm3.h"

#ifdef NVMCOMM3

// Public
// TODO: should go to vm.c
s08_t g_nvc3_file_open = -1;
u16_t g_nvc3_file_pos = 0;

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

void nvmcomm_send(address_t dest, u08_t *payload, u08_t length) {
  nvmcomm_zwave_send(dest, payload, length, 0);
}
// Private

void handle_message(address_t src, u08_t *payload, u08_t length) {
  // TODO: temporary until we figure out where the extra 'packets' from the Z-Wave module are coming from
  if (payload[0] != 0x42 && payload[1] != 0x42) {
    DEBUGF_COMM("====discarded because of missing 0x42 0x42====\n");
    return;
  }
  DEBUGF_COMM("====accepted: "DBG8" "DBG8"====\n", payload[0], payload[1]);  
  // Skip 0x42 0x42
  payload = payload+2;
  length = length-2;
  // END TODO

  const u08_t command = payload[0];
  u08_t response_size = 0;

#ifdef DEBUG
  DEBUGF_COMM("Handling message from "DBG8", length "DBG8": ", src, length);
  for (size8_t i=0; i<length; ++i) {
    DEBUGF_COMM(" "DBG8"", payload[i]);
  }
  DEBUGF_COMM("\n");
#endif
  
  switch (command) {
    case NVC3_CMD_FOPEN:
      if (payload[1] <= NVC3_MAX_FID) {
        DEBUGF_COMM("Open file "DBG8"\n", payload[1]);
        g_nvc3_file_open = payload[1];
        g_nvc3_file_pos = 0;
      }
    break;
    case NVC3_CMD_FCLOSE: 
      DEBUGF_COMM("Closing file\n");
      g_nvc3_file_open = -1;
    break;
    case NVC3_CMD_FSEEK:
      g_nvc3_file_pos = ((u16_t)payload[1]<<8) + payload[2];
      DEBUGF_COMM("Seek to position "DBG16"\n", g_nvc3_file_pos);
    break;
    case NVC3_CMD_RDFILE:
      if (g_nvc3_file_open == NVC3_FILE_FIRMWARE) {
        u08_t *addr = nvmfile_get_base();
        addr += g_nvc3_file_pos;

        response_size = payload[1];
        if (response_size < 39) { // TODO: check for buffer size (depends on protocol)
          for (size8_t i=0; i<response_size; ++i) {
            payload[i] = nvmfile_read08(addr++);
            ++g_nvc3_file_pos;
          }
        }
      }
    break;
    case NVC3_CMD_WRFILE: 
      if (g_nvc3_file_open == NVC3_FILE_FIRMWARE) {
        vm_set_runlevel(NVM_RUNLVL_CONF); // opening firmware for writing implies conf runlevel

        u08_t *addr = nvmfile_get_base();
        addr += g_nvc3_file_pos;

        DEBUGF_COMM("Write "DBG8" bytes at position "DBG16", address "DBG16":", length-1, g_nvc3_file_pos, addr);
        
        for (size8_t i=1; i<length; ++i) {
          DEBUGF_COMM(" "DBG8"", payload[i]);
          nvmfile_write08(addr++, payload[i]);
          ++g_nvc3_file_pos;
        }
        DEBUGF_COMM("\nDone\n");
      }
    break;
    case NVC3_CMD_GETRUNLVL: 
      payload[0] = nvm_runlevel;
      response_size = 1;
    break;
    case NVC3_CMD_SETRUNLVL:
      DEBUGF_COMM("Goto runlevel "DBG8"\n", payload[1]);
      vm_set_runlevel(payload[1]);
    break;
  }
  if (response_size > 0) {
    nvmcomm_send(src, payload, response_size);
  }
}

#endif // NVMCOMM3
