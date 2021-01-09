# Dancy Operating System

##############################################################################

DANCY_KERNEL_OBJECTS_32= \
 ./o32/acpios/a32/glblock.o \
 ./o32/base/apic.o \
 ./o32/base/delay.o \
 ./o32/base/start.o \
 ./o32/common/crc32.o \
 ./o32/common/crc32c.o \
 ./o32/lib/a32/cpu.o \
 ./o32/lib/a32/dark.o \
 ./o32/lib/a32/spin.o \
 ./o32/lib/a32/string.o \
 ./o32/lib/bitarray.o \
 ./o32/lib/ctype.o \
 ./o32/lib/huffman.o \
 ./o32/lib/inflate.o \
 ./o32/lib/snprintf.o \
 ./o32/lib/stdlib.o \
 ./o32/lib/string.o

DANCY_KERNEL_OBJECTS_64= \
 ./o64/acpios/a64/glblock.o \
 ./o64/base/apic.o \
 ./o64/base/delay.o \
 ./o64/base/start.o \
 ./o64/common/crc32.o \
 ./o64/common/crc32c.o \
 ./o64/lib/a64/cpu.o \
 ./o64/lib/a64/dark.o \
 ./o64/lib/a64/spin.o \
 ./o64/lib/a64/string.o \
 ./o64/lib/bitarray.o \
 ./o64/lib/ctype.o \
 ./o64/lib/huffman.o \
 ./o64/lib/inflate.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/stdlib.o \
 ./o64/lib/string.o

##############################################################################

./o32/kernel.at: $(DANCY_KERNEL_OBJECTS_32)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_KERNEL_OBJECTS_32)

./o64/kernel.at: $(DANCY_KERNEL_OBJECTS_64)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_KERNEL_OBJECTS_64)
