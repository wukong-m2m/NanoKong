#
# Makefile include for NanoVM core files
#

AVR_DIR = $(ROOT_DIR)/vm/src/avr
AVR_OBJS = \
  native_avr.o \
  native_impl.o \
  avr_flash.o

OBJS += $(AVR_OBJS)

# convert and upload a class file

%.o:$(AVR_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.d:$(AVR_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's#\($*\)\.o[ :]*#\1.o $@ : #g' > $@ ; \
	[ -s $@ ] || rm -f $@
