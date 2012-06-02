#
#  Makefile for Mega8 test version of the NanoVM
#

CONFIG = Mega8.config
DEVICE = atmega2560

ifdef FLOWXML
DEFAULT_CLASS=$(FLOWXML)
# Default do discovery, unless overridden by a commandline parameter, in which case this gets ignored
DISCOVERY_FLAGS=-D
else

DEFAULT_CLASS = TestWKPF
#DEFAULT_CLASS = AdcDemo
#DEFAULT_CLASS = HAScenario1

endif

PORT = /dev/ttyACM0

ROOT_DIR = ../../..
FLOWXML_DIR = $(ROOT_DIR)/Applications
COMPONENTXML_PATH = $(ROOT_DIR)/ComponentDefinitions/WuKongStandardLibrary.xml
NVMCOMM_REPROGRAM = $(ROOT_DIR)/tools/python/reprogram.py

# general GCC compiler flags
CFLAGS = -std=gnu99 -Wall -Werror -pedantic-errors -Wundef -Wno-long-long -Winline

# AVR compiler setup
#PREFIX = /Applications/Arduino.app/Contents/Resources/Java/hardware/tools/avr/bin/
CC = $(PREFIX)avr-gcc
OBJCOPY = $(PREFIX)avr-objcopy
OBJDUMP = $(PREFIX)avr-objdump

# AVR compiler and linker flags
CFLAGS += -Os -ffunction-sections -DAVR -I. -mmcu=$(DEVICE) -DVERSION="\"$(VERSION)\""

CLDFLAGS += -Wl,--gc-sections,-Tavr4_nanokong.x,--section-start=.reprogram_flash_page=0x3F000 -mmcu=$(DEVICE) # Arduino bootloader without "monitor" ends around 0x3E900, place our code after the Arduino BL

# build intel hex files
all: NanoVM.hex NanoVM_eeprom.hex NanoVM_NRWWsection.hex

include ../../src/Makefile
include ../../src/avr/Makefile
include ../../src/tests/Makefile
include ../../src/native_wuclasses/Makefile

nvmdefault.h: Makefile

# define canned command sequences
define COMPILE-CMD
$(CC) $(CFLAGS) -c $< -o $@
endef

define DEP-GEN-CMD
set -e; $(CC) -MM $(CFLAGS) $< \
| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@ ; \
[ -s $@ ] || rm -f $@
endef

%.o: %.c Makefile
	$(COMPILE-CMD)

%.d:%.c
	$(DEP-GEN-CMD)

# avr specific entries
%.elf: generate $(OBJS)
	$(CC) $(CLDFLAGS) -o $@ $(OBJS)
	avr-size -A $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .lowtext -j .data -j .javabytecode -O ihex $< $@

%_NRWWsection.hex: %.elf
	$(OBJCOPY) -j .reprogram_flash_page -O ihex $< $@

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom -O ihex $< $@

# use uisp tool to configure whole cpu
uisp: NanoVM.hex NanoVM_eeprom.hex
	uisp  -dprog=stk200 --erase  --wr_fuse_l=0xb4 --wr_fuse_h=0xd9
	uisp --segment=flash  -dprog=stk200 --upload if=NanoVM.hex --verify
	uisp --segment=eeprom  -dprog=stk200 --upload if=NanoVM_eeprom.hex --verify

nvmcomm_reprogram: generate nvmdefault.h
	$(NVMCOMM_REPROGRAM) 3 nvmdefault.h
	$(NVMCOMM_REPROGRAM) 1 nvmdefault.h

avrdude: NanoVM.hex NanoVM_eeprom.hex
	avrdude -V -p atmega2560 -P $(PORT) -c stk500v2 -U flash:w:NanoVM.hex -U eeprom:w:NanoVM_eeprom.hex

avrdude_screen: NanoVM.hex NanoVM_eeprom.hex
	avrdude -V -p atmega2560 -P $(PORT) -c stk500v2 -U flash:w:NanoVM.hex -U eeprom:w:NanoVM_eeprom.hex
	screen $(PORT) 115200

clean:
	rm -f *.d *.o *~ nvmdefault.h *.elf avr/*.d *.hex

upload_bootloader:
	avrdude -p atmega2560 -P usb -c usbtiny -e    -U lock:w:0x3F:m -U efuse:w:0xFD:m -U hfuse:w:0xD8:m -U lfuse:w:0xFF:m # Erase and set bits so we can write to NRWW section
	avrdude -p atmega2560 -P usb -c usbtiny -D -V -U flash:w:../../../tools/arduino-stk500v2-bootloader/stk500boot_v2_mega2560.hex:i # Flash the Arduino bootloader. Don't erase. Don't verify until we figure out what's wrong with TINYusbISP reading flash memory, since it will fail anyway.
	avrdude -p atmega2560 -P usb -c usbtiny -D -V -U flash:w:NanoVM_NRWWsection.hex:i # Add our own code in the NRWW section. Don't erase. Don't verify until we figure out what's wrong with TINYusbISP reading flash memory, since it will fail anyway.
	avrdude -p atmega2560 -P usb -c usbtiny       -U lock:w:0x0F:m -U hfuse:w:0xD0:m # Restore lock and hfuse to Arduino default setting
  
generate: $(COMPONENTXML_PATH)
	$(ROOT_DIR)/tools/python/codegen.py -i $(COMPONENTXML_PATH) -p $(ROOT_DIR) -u niagara

#include $(OBJS:.o=.d)

.PHONY : generate
