#
# Makefile include for NanoVM core files
#

NATIVE_PROFILES_DIR = $(ROOT_DIR)/vm/src/native_profiles
NATIVE_PROFILES_OBJS = \
    native_profiles/native_profiles.o \
    native_profiles/profile_generic.o \
    native_profiles/profile_dummy.o

OBJS += $(NATIVE_PROFILES_OBJS)

# convert and upload a class file

native_profiles/%.o:$(NATIVE_PROFILES_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

native_profiles/%.d:$(NATIVE_PROFILES_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's#\($*\)\.o[ :]*#\1.o $@ : #g' > $@ ; \
	[ -s $@ ] || rm -f $@
