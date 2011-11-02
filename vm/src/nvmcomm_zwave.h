#ifndef NVMCOMM_ZWAVE_H
#define NVMCOMM_ZWAVE_H

#include "types.h"

#define TRANSMIT_OPTION_ACK                                0x01   //request acknowledge from destination node
#define TRANSMIT_OPTION_LOW_POWER                          0x02   // transmit at low output power level (1/3 of normal RF range)
#define TRANSMIT_OPTION_RETURN_ROUTE                       0x04   // request transmission via return route 
#define TRANSMIT_OPTION_AUTO_ROUTE                         0x04   // request retransmission via repeater nodes 
#define TRANSMIT_OPTION_NO_ROUTE                           0x10   // do not use response route - Even if available 


// Initialise the Z-Wave interface
extern void nvmcomm_zwave_init(void);
// Register the callback function when we receive packet from the Z-Wave module
extern void nvmcomm_zwave_setcallback(void (*func)(address_t, u08_t, u08_t *, u08_t));
// Call this periodically to receive data
extern void nvmcomm_zwave_poll(void);
// Send ZWave command to another node. This command can be used as wireless repeater between 
// two nodes. It has no assumption of the payload sent between them.
extern int nvmcomm_zwave_send(address_t dest, u08_t nvc3_command, u08_t *b, u08_t l, u08_t option);

#endif // NVMCOMM_ZWAVE_H