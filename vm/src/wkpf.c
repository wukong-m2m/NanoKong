#include "error.h"
#include "debug.h"
#include "wkpf.h"

#include "native_wuclasses/native_wuclasses.h"

void wkpf_init() {
  uint8_t retval;
  retval = wkpf_native_wuclasses_init();
  if (retval != WKPF_OK) {
    DEBUGF_WKPF("WKPF: Error while initialising native wuclasses: %x\n", retval);
    error(ERROR_WKPF_INIT_FAILED);
  }
}
