# Dancy Operating System

##############################################################################

./o32/bitarray/bitarray.o: ./common/bitarray/bitarray.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/bitarray/bitarray.c

./o32/crc32/crc32.o: ./common/crc32/crc32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32/crc32.c

./o32/crc32/crc32c.o: ./common/crc32/crc32c.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32/crc32c.c

./o32/huffman/huffman.o: ./common/huffman/huffman.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/huffman/huffman.c

./o32/core/start.o: ./kernel/core/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/core/start.c

./o32/init/a32/cpu.o: ./kernel/init/a32/cpu.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/cpu.asm

./o32/init/a32/start.o: ./kernel/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/start.asm

./o32/init/a32/syscall.o: ./kernel/init/a32/syscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/syscall.asm

./o32/init/acpi.o: ./kernel/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/acpi.c

./o32/init/cpu.o: ./kernel/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/cpu.c

./o32/init/db.o: ./kernel/init/db.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/db.c

./o32/init/inflate.o: ./kernel/init/inflate.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/inflate.c

./o32/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/init.c

./o32/init/ld.o: ./kernel/init/ld.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/ld.c

./o32/init/log.o: ./kernel/init/log.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/log.c

./o32/init/memory.o: ./kernel/init/memory.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/memory.c

./o32/init/print.o: ./kernel/init/print.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/print.c

./o32/init/rtc.o: ./kernel/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/rtc.c

./o32/lib/a32/dark.o: ./kernel/lib/a32/dark.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/dark.asm

./o32/lib/a32/string.o: ./kernel/lib/a32/string.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/string.asm

./o32/lib/ctype.o: ./kernel/lib/ctype.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/ctype.c

./o32/lib/snprintf.o: ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/snprintf.c

./o32/lib/stdlib.o: ./kernel/lib/stdlib.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/stdlib.c

./o32/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/string.c

./o64/bitarray/bitarray.o: ./common/bitarray/bitarray.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/bitarray/bitarray.c

./o64/crc32/crc32.o: ./common/crc32/crc32.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32/crc32.c

./o64/crc32/crc32c.o: ./common/crc32/crc32c.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32/crc32c.c

./o64/huffman/huffman.o: ./common/huffman/huffman.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/huffman/huffman.c

./o64/core/start.o: ./kernel/core/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/core/start.c

./o64/init/a64/cpu.o: ./kernel/init/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/cpu.asm

./o64/init/a64/start.o: ./kernel/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/start.asm

./o64/init/a64/syscall.o: ./kernel/init/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/syscall.asm

./o64/init/acpi.o: ./kernel/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/acpi.c

./o64/init/cpu.o: ./kernel/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/cpu.c

./o64/init/db.o: ./kernel/init/db.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/db.c

./o64/init/inflate.o: ./kernel/init/inflate.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/inflate.c

./o64/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/init.c

./o64/init/ld.o: ./kernel/init/ld.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/ld.c

./o64/init/log.o: ./kernel/init/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/log.c

./o64/init/memory.o: ./kernel/init/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/memory.c

./o64/init/print.o: ./kernel/init/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/print.c

./o64/init/rtc.o: ./kernel/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/rtc.c

./o64/lib/a64/dark.o: ./kernel/lib/a64/dark.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/dark.asm

./o64/lib/a64/string.o: ./kernel/lib/a64/string.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/string.asm

./o64/lib/ctype.o: ./kernel/lib/ctype.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/ctype.c

./o64/lib/snprintf.o: ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/snprintf.c

./o64/lib/stdlib.o: ./kernel/lib/stdlib.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/stdlib.c

./o64/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/string.c

./o64/uefi/a64/file.o: ./kernel/uefi/a64/file.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/uefi/a64/file.asm

./o64/uefi/a64/memory.o: ./kernel/uefi/a64/memory.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/uefi/a64/memory.asm

./o64/uefi/a64/start.o: ./kernel/uefi/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/uefi/a64/start.asm

./o64/uefi/a64/syscall.o: ./kernel/uefi/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/uefi/a64/syscall.asm

./o64/uefi/file.o: ./kernel/uefi/file.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/file.c

./o64/uefi/log.o: ./kernel/uefi/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/log.c

./o64/uefi/memory.o: ./kernel/uefi/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/memory.c

./o64/uefi/misc.o: ./kernel/uefi/misc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/misc.c

./o64/uefi/print.o: ./kernel/uefi/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/print.c

./o64/uefi/syscall.o: ./kernel/uefi/syscall.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/syscall.c

./o64/uefi/uefi.o: ./kernel/uefi/uefi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/uefi/uefi.c
