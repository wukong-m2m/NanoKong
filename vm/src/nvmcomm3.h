#ifndef NVMCOMM3_H
#define NVMCOMM3_H

#ifdef NVMCOMM3

#define NVC3_CMD_FOPEN        0x70
#define NVC3_CMD_FCLOSE       0x72
#define NVC3_CMD_FSEEK        0x74
#define NVC3_CMD_RDFILE       0x76
#define NVC3_CMD_RDFILE_R     0x77
#define NVC3_CMD_WRFILE       0x78
#define NVC3_CMD_GETRUNLVL    0x7a
#define NVC3_CMD_GETRUNLVL_R  0x7b
#define NVC3_CMD_SETRUNLVL    0x7c
#define NVC3_CMD_SETRUNLVL_R  0x7d
#define NVC3_CMD_APPMSG       0x7e
#define NVC3_CMD_APPMSG_R     0x7f


#define NVC3_CMD_APPMSG_WAIT_ACK   0x00
#define NVC3_CMD_APPMSG_ACK        0x01
#define NVC3_CMD_APPMSG_BUSY       0x02

#define NVC3_MAX_FID 0           // maximum supported file id
#define NVC3_FILE_FIRMWARE 0x00  // firmware file id
#define NVC3_FILE_DATA 0x01  // data file id (not yet supported)

#define NVC3_MESSAGE_SIZE   0x20

// Initialise nvmcomm and whatever protocol is enabled.
extern void nvmcomm_init(void);
// Send length bytes to dest
extern int nvmcomm_send(address_t dest, u08_t nvc3_command, u08_t *payload, u08_t length);
// Call this periodically to receive data
extern void nvmcomm_poll(void);

extern uint8_t nvc3_appmsg_buf[NVC3_MESSAGE_SIZE];
extern uint8_t nvc3_appmsg_size;


#endif // NVMCOMM3

#endif // NVMCOMM3_H