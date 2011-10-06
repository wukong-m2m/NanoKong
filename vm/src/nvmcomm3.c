#include "nvmcomm3.h"

#include "types.h"
#include "debug.h"
#ifdef NVM_USE_COMMZWAVE
#include "nvmcomm_zwave.h"
#endif

#ifdef NVMCOMM3

// Public
void x(u08_t *payload, u08_t length);
void nvmcomm_init(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_init();
  nvmcomm_zwave_setcallback(x);
#endif
}

void nvmcomm_poll(void) {
#ifdef NVM_USE_COMMZWAVE
  nvmcomm_zwave_poll();
#endif
}

// Private
void x(u08_t *payload, u08_t length) {
  DEBUGF_COMM("NVMCOMM3 - Packet received with length %d\n", length);
  for(u08_t i=0; i<length; i++) {
    DEBUGF_COMM("NVMCOMM3 - Byte %d: %d\n", i, payload[i]);
  }
}

#endif // NVMCOMM3
