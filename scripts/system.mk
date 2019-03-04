# Dancy Operating System

##############################################################################

./external/external.sh:
	@bash ./scripts/external.sh

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/bitarray/bitarray.o \
 ./o32/crc32/crc32.o \
 ./o32/crc32/crc32c.o \
 ./o32/huffman/huffman.o \
 ./o32/init/a32/cpu.o \
 ./o32/init/a32/int20h.o \
 ./o32/init/a32/start.o \
 ./o32/init/acpi.o \
 ./o32/init/bprint.o \
 ./o32/init/cpu.o \
 ./o32/init/inflate.o \
 ./o32/init/init.o \
 ./o32/init/log.o \
 ./o32/init/memory.o \
 ./o32/init/rtc.o \
 ./o32/lib/a32/string.o \
 ./o32/lib/snprintf.o \
 ./o32/lib/string.o

DANCY_INIT_OBJECTS_64= \
 ./o64/bitarray/bitarray.o \
 ./o64/crc32/crc32.o \
 ./o64/crc32/crc32c.o \
 ./o64/huffman/huffman.o \
 ./o64/init/a64/cpu.o \
 ./o64/init/a64/int20h.o \
 ./o64/init/a64/start.o \
 ./o64/init/acpi.o \
 ./o64/init/bprint.o \
 ./o64/init/cpu.o \
 ./o64/init/inflate.o \
 ./o64/init/init.o \
 ./o64/init/log.o \
 ./o64/init/memory.o \
 ./o64/init/rtc.o \
 ./o64/lib/a64/string.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/string.o

./system/CONFIG.AT: ./bin/dy-conf$(DANCY_EXE)
	$(DY_CONF) -o$@

./system/IN_IA32.AT: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -finit $(DANCY_INIT_OBJECTS_32)
	$(DY_INIT) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -finit $(DANCY_INIT_OBJECTS_64)
	$(DY_INIT) -tx64 --set-header $@

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tloader $@
