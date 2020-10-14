# Dancy Operating System

##############################################################################

DANCY_START_OBJECTS_32= \
 ./o32/common/crc32.o \
 ./o32/common/crc32c.o \
 ./o32/init/a32/ld.o \
 ./o32/init/a32/start.o \
 ./o32/init/a32/syscall.o \
 ./o32/init/db.o \
 ./o32/init/ld.o \
 ./o32/init/memory.o \
 ./o32/init/print.o \
 ./o32/init/start.o \
 ./o32/lib/a32/dark.o \
 ./o32/lib/a32/string.o \
 ./o32/lib/bitarray.o \
 ./o32/lib/huffman.o \
 ./o32/lib/inflate.o \
 ./o32/lib/snprintf.o \
 ./o32/lib/string.o

DANCY_START_OBJECTS_64= \
 ./o64/common/crc32.o \
 ./o64/common/crc32c.o \
 ./o64/init/a64/ld.o \
 ./o64/init/a64/start.o \
 ./o64/init/a64/syscall.o \
 ./o64/init/db.o \
 ./o64/init/ld.o \
 ./o64/init/memory.o \
 ./o64/init/print.o \
 ./o64/init/start.o \
 ./o64/lib/a64/dark.o \
 ./o64/lib/a64/string.o \
 ./o64/lib/bitarray.o \
 ./o64/lib/huffman.o \
 ./o64/lib/inflate.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/string.o

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/common/fat.o \
 ./o32/common/ttf.o \
 ./o32/init/a32/cpu.o \
 ./o32/init/a32/gdt.o \
 ./o32/init/a32/idt.o \
 ./o32/init/acpi.o \
 ./o32/init/cpu.o \
 ./o32/init/fs.o \
 ./o32/init/gui.o \
 ./o32/init/idt.o \
 ./o32/init/init.o \
 ./o32/init/log.o \
 ./o32/init/rtc.o \
 ./o32/init/vga.o

DANCY_INIT_OBJECTS_64= \
 ./o64/common/fat.o \
 ./o64/common/ttf.o \
 ./o64/init/a64/cpu.o \
 ./o64/init/a64/gdt.o \
 ./o64/init/a64/idt.o \
 ./o64/init/acpi.o \
 ./o64/init/cpu.o \
 ./o64/init/fs.o \
 ./o64/init/gui.o \
 ./o64/init/idt.o \
 ./o64/init/init.o \
 ./o64/init/log.o \
 ./o64/init/rtc.o \
 ./o64/init/vga.o

##############################################################################

./o32/init.at: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_32)

./o64/init.at: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_64)
