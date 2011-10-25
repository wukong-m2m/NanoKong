#include "native.h"
#include "stack.h"
#include "debug.h"
#include "error.h"

#include "native_nvmcomm3.h"

void native_nvmcomm3_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_SEND) {
    u08_t dest = (u08_t)stack_pop_int();
    DEBUGF_COMM("native send to "DBG8"\n", dest);
  } else if(mref == NATIVE_METHOD_RECEIVE) {
    nvm_int_t waitmsec=stack_pop_int();
    DEBUGF_COMM("native receive "DBG16"\n", waitmsec);
  } else
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}
