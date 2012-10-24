#ifndef NVMCOMM_H
#define NVMCOMM_H

#include "config.h"
#ifdef NVM_USE_COMMZWAVE
#include "nvmcomm_zwave.h"
#endif
#ifdef NVM_USE_COMMXBEE
#include "nvmcomm_xbee.h"
#endif

#ifdef NVM_USE_COMM

#include "types.h"

#define NVMCOMM_CMD_REPRG_OPEN                   0x10
#define NVMCOMM_CMD_REPRG_OPEN_R                 0x11
#define NVMCOMM_CMD_REPRG_WRITE                  0x20
#define NVMCOMM_CMD_REPRG_WRITE_R_OK             0x21
#define NVMCOMM_CMD_REPRG_WRITE_R_RETRANSMIT     0x22
#define NVMCOMM_CMD_REPRG_COMMIT                 0x30
#define NVMCOMM_CMD_REPRG_COMMIT_R_OK            0x31
#define NVMCOMM_CMD_REPRG_COMMIT_R_RETRANSMIT    0x32
#define NVMCOMM_CMD_REPRG_COMMIT_R_FAILED        0x33

#define NVMCOMM_CMD_GETRUNLVL                    0x50
#define NVMCOMM_CMD_GETRUNLVL_R                  0x51
#define NVMCOMM_CMD_SETRUNLVL                    0x60
#define NVMCOMM_CMD_SETRUNLVL_R                  0x61

#define NVMCOMM_CMD_APPMSG                       0x80
#define NVMCOMM_CMD_APPMSG_R                     0x81

#define NVMCOMM_APPMSG_WAIT_ACK                  0x00
#define NVMCOMM_APPMSG_ACK                       0x01
#define NVMCOMM_APPMSG_BUSY                      0x02

#define NVMCOMM_GROUP_PROPOSE                    0x70
#define NVMCOMM_GROUP_PROPOSE_R                  0x71
#define NVMCOMM_GROUP_COMMIT                     0x72
#define NVMCOMM_GROUP_EVENT_JOIN                 0x74
#define NVMCOMM_GROUP_EVENT_JOIN_R               0x75
#define NVMCOMM_GROUP_EVENT_LEAVE                0x76
#define NVMCOMM_GROUP_EVENT_LEAVE_R              0x77
#define NVMCOMM_GROUP_EVENT_ORACLE_FIND          0x78
#define NVMCOMM_GROUP_EVENT_ORACLE_FIND_R        0x79
#define NVMCOMM_GROUP_EVENT_FULL_R               0x7A
#define NVMCOMM_GROUP_ERROR_R                    0x7F

#define NVMCOMM_WKPF_GET_WUCLASS_LIST            0x90
#define NVMCOMM_WKPF_GET_WUCLASS_LIST_R          0x91
#define NVMCOMM_WKPF_GET_WUOBJECT_LIST           0x92
#define NVMCOMM_WKPF_GET_WUOBJECT_LIST_R         0x93
#define NVMCOMM_WKPF_READ_PROPERTY               0x94
#define NVMCOMM_WKPF_READ_PROPERTY_R             0x95
#define NVMCOMM_WKPF_WRITE_PROPERTY              0x96
#define NVMCOMM_WKPF_WRITE_PROPERTY_R            0x97
#define NVMCOMM_WKPF_REQUEST_PROPERTY_INIT       0x98
#define NVMCOMM_WKPF_REQUEST_PROPERTY_INIT_R     0x99
#define NVMCOMM_WKPF_GET_LOCATION                0x9A
#define NVMCOMM_WKPF_GET_LOCATION_R              0x9B
#define NVMCOMM_WKPF_SET_LOCATION                0x9C
#define NVMCOMM_WKPF_SET_LOCATION_R              0x9D
#define NVMCOMM_WKPF_ERROR_R                     0x9F

#define NVMCOMM_DEBUG_TRACE_PART                 0xA0
#define NVMCOMM_DEBUG_TRACE_FINAL                0xA2

//#define NVMCOMM_WKPF_...

#define NVMCOMM_MAX_FID 0           // maximum supported file id
#define NVMCOMM_FILE_FIRMWARE 0x00  // firmware file id
#define NVMCOMM_FILE_DATA 0x01  // data file id (not yet supported)

#define NVMCOMM_MESSAGE_SIZE   0x20

#define NVMCOMM_ADDRESS_NULL 0xff

typedef struct nvmcomm_message {
  address_t from;
  address_t to;
  u08_t command;
  u08_t *payload;
  u08_t payload_length;
} nvmcomm_message;


// Initialise nvmcomm and whatever protocol is enabled.
extern void nvmcomm_init(void);
// Send length bytes to dest
extern int nvmcomm_send(address_t dest, u08_t nvc3_command, u08_t *payload, u08_t length);
// Multicast length bytes to dests, no MAC acks
extern int nvmcomm_multicast(address_t* dests, u08_t dest_size, u08_t nvc3_command, u08_t *payload, u08_t length);
// Broadcast length bytes
extern int nvmcomm_broadcast(u08_t nvc3_command, u08_t *payload, u08_t length);
// Call this periodically to receive data
extern void nvmcomm_poll(void);
// Wait for a message of a specific type, while still handling messages of other types
extern nvmcomm_message *nvmcomm_wait(u16_t wait_msec, u08_t *commands, u08_t number_of_commands);
// Get my own node id
extern address_t nvmcomm_get_node_id();

extern uint8_t nvc3_appmsg_buf[NVMCOMM_MESSAGE_SIZE];
extern uint8_t nvc3_appmsg_size;

extern void set_message_sequence_number(u08_t* payload, u08_t* payload_length);
extern bool_t check_sequence_number(u08_t* first_payload, u08_t* second_payload);

#endif // NVM_USE_COMM

#endif // NVMCOMM_H
