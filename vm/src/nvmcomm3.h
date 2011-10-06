#ifndef NVMCOMM3_H
#define NVMCOMM3_H

// Initialise nvmcomm and whatever protocol is enabled.
extern void nvmcomm_init(void);
// Call this periodically to receive data
extern void poll(void);

#endif // NVMCOMM3_H