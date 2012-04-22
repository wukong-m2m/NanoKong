#include "error.h"
#include "debug.h"
#include "wkpf.h"

#include "native_profiles/native_profiles.h"

void wkpf_init() {
  uint8_t retval;
  retval = native_profiles_init();
  if (retval != WKPF_OK) {
    DEBUGF_WKPF("WKPF: Error while initialising native profiles: %x\n", retval);
    error(ERROR_WKPF_INIT_FAILED);
  }
}
