# Dancy Operating System

##############################################################################

./o32/common/bitarray.o: ./common/bitarray/bitarray.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/bitarray/bitarray.c

./o32/common/huffman.o: ./common/huffman/huffman.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/huffman/huffman.c

./o32/init/a32/bsyscall.o: ./kernel/init/a32/bsyscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/bsyscall.asm

./o32/init/a32/cpu.o: ./kernel/init/a32/cpu.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/cpu.asm

./o32/init/a32/start.o: ./kernel/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/start.asm

./o32/init/acpi.o: ./kernel/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/acpi.c

./o32/init/bprint.o: ./kernel/init/bprint.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/bprint.c

./o32/init/inflate.o: ./kernel/init/inflate.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/inflate.c

./o32/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/init.c

./o32/lib/a32/string.o: ./kernel/lib/a32/string.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/string.asm

./o32/lib/snprintf.o: ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/snprintf.c

./o32/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/string.c

./o64/common/bitarray.o: ./common/bitarray/bitarray.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/bitarray/bitarray.c

./o64/common/huffman.o: ./common/huffman/huffman.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/huffman/huffman.c

./o64/init/a64/bsyscall.o: ./kernel/init/a64/bsyscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/bsyscall.asm

./o64/init/a64/cpu.o: ./kernel/init/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/cpu.asm

./o64/init/a64/start.o: ./kernel/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/start.asm

./o64/init/acpi.o: ./kernel/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/acpi.c

./o64/init/bprint.o: ./kernel/init/bprint.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/bprint.c

./o64/init/inflate.o: ./kernel/init/inflate.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/inflate.c

./o64/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/init.c

./o64/lib/a64/string.o: ./kernel/lib/a64/string.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/string.asm

./o64/lib/snprintf.o: ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/snprintf.c

./o64/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/string.c
