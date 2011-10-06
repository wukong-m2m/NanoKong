#ifndef NVMCOMM_ZWAVE_H
#define NVMCOMM_ZWAVE_H

#include "types.h"

// Initialise the Z-Wave interface
extern void nvmcomm_zwave_init(void);
// Register the callback function when we receive packet from the Z-Wave module
extern void nvmcomm_zwave_setcallback(void (*func)(u08_t *, u08_t));
// Call this periodically to receive data
extern void nvmcomm_zwave_poll(void);
// Send ZWave command to another node. This command can be used as wireless repeater between 
// two nodes. It has no assumption of the payload sent between them.
extern void nvmcomm_zwave_send(u08_t id, u08_t *b, u08_t l, u08_t option);

#endif // NVMCOMM_ZWAVE_H