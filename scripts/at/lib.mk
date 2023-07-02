# Dancy Operating System

##############################################################################

DANCY_LIB_OBJECTS_32= \
 ./o32/common/a32/dark.o \
 ./o32/common/a32/string.o \
 ./o32/common/epoch.o \
 ./o32/common/utf8.o \
 ./o32/kernel/lib/a32/coff.o \
 ./o32/kernel/lib/a32/cpu.o \
 ./o32/kernel/lib/a32/spin.o \
 ./o32/kernel/lib/bitarray.o \
 ./o32/kernel/lib/ctype.o \
 ./o32/kernel/lib/huffman.o \
 ./o32/kernel/lib/inflate.o \
 ./o32/kernel/lib/snprintf.o \
 ./o32/kernel/lib/stdlib.o \
 ./o32/kernel/lib/strerror.o \
 ./o32/kernel/lib/string.o \

DANCY_LIB_OBJECTS_64= \
 ./o64/common/a64/dark.o \
 ./o64/common/a64/string.o \
 ./o64/common/epoch.o \
 ./o64/common/utf8.o \
 ./o64/kernel/lib/a64/coff.o \
 ./o64/kernel/lib/a64/cpu.o \
 ./o64/kernel/lib/a64/spin.o \
 ./o64/kernel/lib/bitarray.o \
 ./o64/kernel/lib/ctype.o \
 ./o64/kernel/lib/huffman.o \
 ./o64/kernel/lib/inflate.o \
 ./o64/kernel/lib/snprintf.o \
 ./o64/kernel/lib/stdlib.o \
 ./o64/kernel/lib/strerror.o \
 ./o64/kernel/lib/string.o \

##############################################################################

./o32/lib.at: $(DANCY_LIB_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_LIB_OBJECTS_32)

./o64/lib.at: $(DANCY_LIB_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_LIB_OBJECTS_64)
