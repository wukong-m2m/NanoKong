#include "nvmcomm3.h"

#include "types.h"
#include "debug.h"
#ifdef NVM_USE_COMMZWAVE
#include "nvmcomm_zwave.h"
#endif

// Public

void nvmcomm_init(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_init();
#endif
}

void nvmcomm_poll(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_poll();
#endif
}

// Private
void x(u08_t *payload, u08_t length) {
  DEBUGF("NVMCOMM3 - Received packet with length %d", length);
  for(u08_t i=0; i<length; i++) {
    DEBUGF("NVMCOMM3 - Byte %d: %d", i, payload[i]);
  }
}


