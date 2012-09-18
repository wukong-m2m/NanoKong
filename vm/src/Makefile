#
# Makefile include for NanoVM core files
#

VERSION = 1.6

NVM_DIR = $(ROOT_DIR)/vm/src
CFLAGS += -I$(NVM_DIR)
NVM_OBJS = NanoVM.o nvmfile.o vm.o heap.o array.o \
	error.o native_stdio.o stack.o \
	uart.o debug.o native_lcd.o \
	native_math.o native_formatter.o nvmstring.o \
	nvmcomm.o nvmcomm_zwave.o native_nvmcomm.o nvmcomm_xbee.o \
	wkpf.o wkpf_properties.o wkpf_wuobjects.o wkpf_wuclasses.o \
	wkpf_links.o wkpf_comm.o native_wkpf.o

OBJS += $(NVM_OBJS)

DEFAULT_FILE = $(EXAMPLES_PATH)/$(DEFAULT_CLASS).java

CLASSES_PATH = $(ROOT_DIR)/java/classes
EXAMPLES_PATH = $(ROOT_DIR)/java/examples
NANOVM_PATH = $(ROOT_DIR)/java/nanovm
WKPF_PATH = $(NANOVM_PATH)/wkpf
JAVA_FILES = $(NANOVM_PATH)/*/*.java

JAR_FILE = NanoKong.jar

ifeq ($(OS),Windows_NT)
PATH_SEP = ;
else
PATH_SEP = :
endif

ifeq ($(COMPILE_LIB),YES)

else
./nvmfile.o: ./nvmdefault.h Makefile
./nvmfile.d: ./nvmdefault.h Makefile
endif

$(DEFAULT_FILE):
ifeq ($(DEFAULT_CLASS), TestWKPF)
	$(ROOT_DIR)/tools/xml2java/translator.py -c $(COMPONENTXML_PATH) -f $(FLOWXML_DIR)/$(FLOWXML).xml -o $(ROOT_DIR)/java/examples $(DISCOVERY_FLAGS)
	cp $(FLOWXML_DIR)/$(FLOWXML).xml $(CURDIR)/currentFlow.xml
	cp $(ROOT_DIR)/java/examples/$(FLOWXML)Mapping.xml $(CURDIR)/currentMapping.xml
endif

$(DEFAULT_CLASS): $(JAVA_FILES) $(DEFAULT_FILE)
	-mkdir -p $(CLASSES_PATH)
	javac -d $(CLASSES_PATH) $^
	#javac -classpath "$(NANOVM_PATH)$(PATH_SEP)$(EXAMPLES_PATH)" $^
	#jar -cf $(JAR_FILE) $(JAVA_FILES:.java=.class)
	#mv $(JAR_FILE) $(EXAMPLES_PATH)

#%.class:%.java
	#javac -classpath "$(NANOVM_PATH)$(PATH_SEP)$(WKPF_PATH)$(PATH_SEP)$(EXAMPLES_PATH)" $<

nvmdefault.h: $(DEFAULT_CLASS)
	java -cp $(ROOT_DIR)/nanovmtool/build/classes org/nanovm/NanoVMTool -c -f $@ $(ROOT_DIR)/nanovmtool/config/$(CONFIG) "$(CLASSES_PATH)" $(DEFAULT_CLASS)


# convert and upload a class file
upload-%: $(ROOT_DIR)/java/examples/%.java
	javac -classpath $(ROOT_DIR)/java$(PATH_SEP)$(ROOT_DIR)/java/examples $(ROOT_DIR)/java/examples/$*.java
	java -jar $(ROOT_DIR)/nanovmtool/NanoVMTool.jar $(ROOT_DIR)/nanovmtool/config/$(CONFIG) $(ROOT_DIR)/java$(PATH_SEP)$(ROOT_DIR)/java/examples $*

%.o: $(NVM_DIR)/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.d: $(NVM_DIR)/%.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@ ; \
	[ -s $@ ] || rm -f $@
