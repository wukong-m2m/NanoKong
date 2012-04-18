#
# Makefile include for NanoVM core files
#

NATIVE_PROFILES_DIR = $(ROOT_DIR)/vm/src/native_profiles
NATIVE_PROFILES_OBJS = \
    native_profiles.o \
    profile_generic.o \
    profile_threshold.o \
    profile_numeric_controller.o \
    profile_light.o \
    profile_light_sensor.o

OBJS += $(NATIVE_PROFILES_OBJS)

# convert and upload a class file

%.o:$(NATIVE_PROFILES_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.d:$(NATIVE_PROFILES_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's#\($*\)\.o[ :]*#\1.o $@ : #g' > $@ ; \
	[ -s $@ ] || rm -f $@
