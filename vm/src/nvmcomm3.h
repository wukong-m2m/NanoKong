#ifndef NVMCOMM3_H
#define NVMCOMM3_H

#ifdef NVMCOMM3

// Initialise nvmcomm and whatever protocol is enabled.
extern void nvmcomm_init(void);
// Call this periodically to receive data
extern void nvmcomm_poll(void);

#endif // NVMCOMM3

#endif // NVMCOMM3_H