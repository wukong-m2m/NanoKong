#
# Makefile include for NanoVM core files
#

TESTS_DIR = $(ROOT_DIR)/vm/src/tests
TESTS_OBJS = \
  test_wkpf.o

OBJS += $(TESTS_OBJS)

# convert and upload a class file

%.o:$(TESTS_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.d:$(TESTS_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's#\($*\)\.o[ :]*#\1.o $@ : #g' > $@ ; \
	[ -s $@ ] || rm -f $@
