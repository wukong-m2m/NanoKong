#include "config.h"
#include "avr_flash.h"
#include "types.h"
#include "debug.h"

#include <string.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

// TODO (if we run short on RAM): this can probably be made more
// efficient by splitting this up in two parts and writing directly
// to the flash page buffer while receiving the code using
// boot_page_fill, and then writing the buffer to flash in a separate
// function.
uint8_t avr_flash_pagebuffer[AVR_FLASH_PAGESIZE];
uint32_t avr_flash_pageaddress = -1;
uint16_t avr_flash_buf_len = 0;

// This function should be in the NRWW section to allow it to write to flash
//void BOOTLOADER_SECTION avr_flash_program_page(uint32_t page, uint8_t *buf);
void __attribute__ ((section (".reprogram_flash_page"))) avr_flash_program_page (uint32_t page, uint8_t *buf);

void avr_flash_clear_buffer() {
	int i;
	for (i=0; i<AVR_FLASH_PAGESIZE; i++)
		avr_flash_pagebuffer[i] = 0xFF;
}

void avr_flash_open(uint32_t address) {
	if (address % AVR_FLASH_PAGESIZE != 0) {
		DEBUGF_FLASH("Flashing to address "DBG32": not a page boundary.");
		avr_flash_pageaddress = -1;
	}
	DEBUGF_FLASH("Start writing to flash at address "DBG32".\n", address);
	avr_flash_clear_buffer();
	avr_flash_pageaddress = address;
	avr_flash_buf_len = 0;
}

void avr_flash_write(uint16_t len, uint8_t *buf) {
	if (avr_flash_pageaddress == -1)
		return;
  DEBUGF_FLASH("Received "DBG8" bytes to flash to page "DBG32".\n", len, avr_flash_pageaddress);
  DEBUGF_FLASH("Buffer already contains "DBG16" bytes.\n", avr_flash_buf_len);
	while(len!=0) {
		uint8_t bytes_on_this_page = len;
		if (avr_flash_buf_len + len > AVR_FLASH_PAGESIZE) // Only 1 page at a time
			bytes_on_this_page = AVR_FLASH_PAGESIZE-avr_flash_buf_len;
		memcpy(avr_flash_pagebuffer + avr_flash_buf_len, buf, bytes_on_this_page); // Copy the data to the page buffer
		if (avr_flash_buf_len + bytes_on_this_page == AVR_FLASH_PAGESIZE) { // If we filled a whole page, write it to flash
  	  DEBUGF_FLASH("Flashing page at "DBG32".\n", avr_flash_pageaddress);
			avr_flash_program_page(avr_flash_pageaddress, avr_flash_pagebuffer);
			avr_flash_clear_buffer();
			avr_flash_pageaddress += AVR_FLASH_PAGESIZE;
		}
		avr_flash_buf_len = (avr_flash_buf_len + bytes_on_this_page) % AVR_FLASH_PAGESIZE;
		len -= bytes_on_this_page;
		buf += bytes_on_this_page;
	}
}

void avr_flash_close() {
	DEBUGF_FLASH("Closing flash file.\n");
	if (avr_flash_buf_len != 0) { // If there's any data remaining, write it to flash.
	  DEBUGF_FLASH("Flashing page at "DBG32".\n", avr_flash_pageaddress);
		avr_flash_program_page(avr_flash_pageaddress, avr_flash_pagebuffer);
	}
	avr_flash_pageaddress = -1;
}


// Copied from avr/boot.h example
void avr_flash_program_page (uint32_t page, uint8_t *buf)
{
	uint16_t i;
	uint8_t sreg;

		// Disable interrupts.

	sreg = SREG;
	cli();

	eeprom_busy_wait ();

	boot_page_erase (page);
	boot_spm_busy_wait ();      // Wait until the memory is erased.

	for (i=0; i<SPM_PAGESIZE; i+=2)
	{
				// Set up little-endian word.

		uint16_t w = *buf++;
		w += (*buf++) << 8;

		boot_page_fill (page + i, w);
	}

	boot_page_write (page);     // Store buffer in flash page.
	boot_spm_busy_wait();       // Wait until the memory is written.

		// Reenable RWW-section again. We need this if we want to jump back
		// to the application after bootloading.

	boot_rww_enable ();

		// Re-enable interrupts (if they were ever enabled).

	SREG = sreg;
}

