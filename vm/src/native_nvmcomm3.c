#include "native.h"
#include "stack.h"
#include "debug.h"
#include "error.h"
#include "native_nvmcomm3.h"
#include "nvmcomm3.h"

void native_nvmcomm3_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_SEND) {
    uint8_t len = (uint8_t)stack_pop_int();
    uint8_t *buf = (uint8_t *)stack_pop_addr();
    uint8_t dest = (uint8_t)stack_pop_int();
    DEBUGF_COMM("native send to "DBG8"\n", dest);
    DEBUGF_COMM(""DBG8" bytes:", len);
    int i;
    for (i=1; i<len+1; i++)
      DEBUGF_COMM("["DBG8"] ", buf[i]);
    DEBUGF_COMM("\n");
    stack_push(nvmcomm_send(dest, buf+1, len)); // +1 to skip the first byte which indicates the type of the array
  } else if(mref == NATIVE_METHOD_RECEIVE) {
    nvm_int_t waitmsec=stack_pop_int();
    DEBUGF_COMM("native receive "DBG16"\n", waitmsec);
  } else
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}
