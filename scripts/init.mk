# Dancy Operating System

##############################################################################

DANCY_START_OBJECTS_32= \
 ./o32/common/crc32.o \
 ./o32/common/crc32c.o \
 ./o32/init/a32/start.o \
 ./o32/init/a32/syscall.o \
 ./o32/init/bitarray.o \
 ./o32/init/db.o \
 ./o32/init/huffman.o \
 ./o32/init/inflate.o \
 ./o32/init/ld.o \
 ./o32/init/memory.o \
 ./o32/init/print.o \
 ./o32/init/start.o \
 ./o32/lib/a32/dark.o \
 ./o32/lib/a32/string.o \
 ./o32/lib/snprintf.o \
 ./o32/lib/string.o

DANCY_START_OBJECTS_64= \
 ./o64/common/crc32.o \
 ./o64/common/crc32c.o \
 ./o64/init/a64/start.o \
 ./o64/init/a64/syscall.o \
 ./o64/init/bitarray.o \
 ./o64/init/db.o \
 ./o64/init/huffman.o \
 ./o64/init/inflate.o \
 ./o64/init/ld.o \
 ./o64/init/memory.o \
 ./o64/init/print.o \
 ./o64/init/start.o \
 ./o64/lib/a64/dark.o \
 ./o64/lib/a64/string.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/string.o

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/init/a32/cpu.o \
 ./o32/init/acpi.o \
 ./o32/init/cpu.o \
 ./o32/init/init.o \
 ./o32/init/log.o \
 ./o32/init/rtc.o

DANCY_INIT_OBJECTS_64= \
 ./o64/init/a64/cpu.o \
 ./o64/init/acpi.o \
 ./o64/init/cpu.o \
 ./o64/init/init.o \
 ./o64/init/log.o \
 ./o64/init/rtc.o

##############################################################################

./o32/init.at: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_32)

./o64/init.at: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_64)
