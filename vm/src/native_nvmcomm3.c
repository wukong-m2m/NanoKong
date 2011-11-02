#include "native.h"
#include "stack.h"
#include "debug.h"
#include "error.h"
#include "array.h"
#include "delay.h"
#include "native_nvmcomm3.h"
#include "nvmcomm3.h"

void native_nvmcomm3_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_SEND) {
    uint8_t len = (uint8_t)stack_pop_int();
    uint8_t *buf = (uint8_t *)stack_pop_addr();
    uint8_t dest = (uint8_t)stack_pop_int();
    DEBUGF_COMM("native send to "DBG8"\n", dest);
    DEBUGF_COMM(""DBG8" bytes:", len);
    for (uint8_t i=1; i<len+1; i++)
      DEBUGF_COMM("["DBG8"] ", buf[i]);
    DEBUGF_COMM("\n");
    stack_push(nvmcomm_send(dest, NVC3_CMD_APPMSG, buf+1, len)); // +1 to skip the first byte which indicates the type of the array
  } else if(mref == NATIVE_METHOD_RECEIVE) {
    nvm_int_t waitmsec=stack_pop_int();
    DEBUGF_COMM("native nvmcomm3.receive: waiting "DBG16" msec\n", waitmsec);
    while (nvc3_appmsg_size==0 && waitmsec>0) {
      delay(MILLISEC(1));
      // Check if there's any packet coming in that we need to handle before processing the next VM instruction.
      // Need to do this here because the VM's main loop is stopped.
      // TODO: Reconsider this when we have a better design for receiving messages.
      nvmcomm_poll();
      waitmsec--;
    }
    if (nvc3_appmsg_size==0) {
      // Nothing was received, return NULL;
      stack_push(0);
      DEBUGF_COMM("native nvmcomm3.receive: timeout\n");
    } else {
      DEBUGF_COMM("native nvmcomm3.receive: received "DBG8" bytes\n", nvc3_appmsg_size);
      // TODO: copy the data to a Java array and return it.
      heap_id_t array = array_new(nvc3_appmsg_size, T_BYTE);
      for (uint8_t i=0; i<nvc3_appmsg_size; i++)
        array_bastore(array, i, nvc3_appmsg_buf[i]);
      stack_push(array | NVM_TYPE_HEAP);
      nvc3_appmsg_size = 0; // Buffer available for next message
    }
  } else
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}
