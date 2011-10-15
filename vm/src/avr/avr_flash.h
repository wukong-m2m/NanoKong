#ifndef AVR_FLASH_H
#define AVR_FLASH_H

#include <avr/boot.h>

#define AVR_FLASH_PAGESIZE SPM_PAGESIZE

void avr_flash_open(uint32_t address); // address%AVR_FLASH_PAGESIZE has to be 0.
void avr_flash_write(uint16_t len, uint8_t *buf);
void avr_flash_close();

#endif // AVR_FLASH_H
