#
# Makefile include for NanoVM core files
#

NATIVE_WUCLASSES_DIR = $(ROOT_DIR)/vm/src/native_wuclasses
NATIVE_WUCLASSES_OBJS = \
    native_wuclasses.o \
    wuclass_generic_update.o \
    wuclass_threshold_update.o \
    wuclass_numeric_controller_update.o \
    wuclass_light_actuator_update.o \
    wuclass_light_sensor_update.o \
    wuclass_logical_update.o \
    wuclass_math_update.o \
    wuclass_loop_delay_boolean_update.o \
    wuclass_loop_delay_short_update.o \
    wuclass_condition_selector_boolean_update.o \
    wuclass_condition_selector_short_update.o \
    wuclass_trigger_update.o \
    GENERATEDwuclass_generic.o \
    GENERATEDwuclass_threshold.o \
    GENERATEDwuclass_numeric_controller.o \
    GENERATEDwuclass_light_actuator.o \
    GENERATEDwuclass_light_sensor.o \
    GENERATEDwuclass_logical.o \
    GENERATEDwuclass_math_op.o \
    GENERATEDwuclass_loop_delay_boolean.o \
    GENERATEDwuclass_loop_delay_short.o \
    GENERATEDwuclass_condition_selector_boolean.o \
    GENERATEDwuclass_condition_selector_short.o \
    GENERATEDwuclass_trigger.o

OBJS += $(NATIVE_WUCLASSES_OBJS)

%.o: $(NATIVE_WUCLASSES_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.d: $(NATIVE_WUCLASSES_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@ ; \
	[ -s $@ ] || rm -f $@
