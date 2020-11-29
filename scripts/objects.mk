# Dancy Operating System

##############################################################################

./o32/common/crc32.o: ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32.c

./o32/common/crc32c.o: ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32c.c

./o32/common/fat.o: ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/fat.c

./o32/common/ttf.o: ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/ttf.c

./o32/acpios/a32/glblock.o: ./kernel/acpios/a32/glblock.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/acpios/a32/glblock.asm

./o32/init/a32/cpu.o: ./kernel/init/a32/cpu.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/cpu.asm

./o32/init/a32/gdt.o: ./kernel/init/a32/gdt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/gdt.asm

./o32/init/a32/idt.o: ./kernel/init/a32/idt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/idt.asm

./o32/init/a32/ld.o: ./kernel/init/a32/ld.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/ld.asm

./o32/init/a32/pg.o: ./kernel/init/a32/pg.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/pg.asm

./o32/init/a32/start.o: ./kernel/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/start.asm

./o32/init/a32/syscall.o: ./kernel/init/a32/syscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/syscall.asm

./o32/init/a32/thrd.o: ./kernel/init/a32/thrd.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/thrd.asm

./o32/init/smp/smp.o: ./kernel/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/smp/smp.c

./o32/init/smp/start32.o: ./kernel/init/smp/start32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/smp/start32.c

./o32/init/acpi.o: ./kernel/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/acpi.c

./o32/init/apic.o: ./kernel/init/apic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/apic.c

./o32/init/cpu.o: ./kernel/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/cpu.c

./o32/init/db.o: ./kernel/init/db.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/db.c

./o32/init/delay.o: ./kernel/init/delay.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/delay.c

./o32/init/fs.o: ./kernel/init/fs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/fs.c

./o32/init/gui.o: ./kernel/init/gui.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/gui.c

./o32/init/hpet.o: ./kernel/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/hpet.c

./o32/init/idt.o: ./kernel/init/idt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/idt.c

./o32/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/init.c

./o32/init/ld.o: ./kernel/init/ld.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/ld.c

./o32/init/log.o: ./kernel/init/log.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/log.c

./o32/init/memory.o: ./kernel/init/memory.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/memory.c

./o32/init/panic.o: ./kernel/init/panic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/panic.c

./o32/init/pg.o: ./kernel/init/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/pg.c

./o32/init/pit.o: ./kernel/init/pit.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/pit.c

./o32/init/print.o: ./kernel/init/print.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/print.c

./o32/init/rtc.o: ./kernel/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/rtc.c

./o32/init/start.o: ./kernel/init/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/start.c

./o32/init/thrd.o: ./kernel/init/thrd.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/thrd.c

./o32/init/vga.o: ./kernel/init/vga.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/vga.c

./o32/lib/a32/dark.o: ./kernel/lib/a32/dark.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/dark.asm

./o32/lib/a32/spin.o: ./kernel/lib/a32/spin.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/spin.asm

./o32/lib/a32/string.o: ./kernel/lib/a32/string.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/string.asm

./o32/lib/bitarray.o: ./kernel/lib/bitarray.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/bitarray.c

./o32/lib/ctype.o: ./kernel/lib/ctype.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/ctype.c

./o32/lib/huffman.o: ./kernel/lib/huffman.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/huffman.c

./o32/lib/inflate.o: ./kernel/lib/inflate.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/inflate.c

./o32/lib/snprintf.o: ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/snprintf.c

./o32/lib/stdlib.o: ./kernel/lib/stdlib.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/stdlib.c

./o32/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/string.c

./o64/common/crc32.o: ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32.c

./o64/common/crc32c.o: ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32c.c

./o64/common/fat.o: ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/fat.c

./o64/common/ttf.o: ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/ttf.c

./o64/acpios/a64/glblock.o: ./kernel/acpios/a64/glblock.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/acpios/a64/glblock.asm

./o64/init/a64/cpu.o: ./kernel/init/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/cpu.asm

./o64/init/a64/gdt.o: ./kernel/init/a64/gdt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/gdt.asm

./o64/init/a64/idt.o: ./kernel/init/a64/idt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/idt.asm

./o64/init/a64/ld.o: ./kernel/init/a64/ld.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/ld.asm

./o64/init/a64/pg.o: ./kernel/init/a64/pg.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/pg.asm

./o64/init/a64/start.o: ./kernel/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/start.asm

./o64/init/a64/syscall.o: ./kernel/init/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/syscall.asm

./o64/init/a64/thrd.o: ./kernel/init/a64/thrd.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/thrd.asm

./o64/init/smp/smp.o: ./kernel/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/smp/smp.c

./o64/init/smp/start64.o: ./kernel/init/smp/start64.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/smp/start64.c

./o64/init/acpi.o: ./kernel/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/acpi.c

./o64/init/apic.o: ./kernel/init/apic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/apic.c

./o64/init/cpu.o: ./kernel/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/cpu.c

./o64/init/db.o: ./kernel/init/db.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/db.c

./o64/init/delay.o: ./kernel/init/delay.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/delay.c

./o64/init/fs.o: ./kernel/init/fs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/fs.c

./o64/init/gui.o: ./kernel/init/gui.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/gui.c

./o64/init/hpet.o: ./kernel/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/hpet.c

./o64/init/idt.o: ./kernel/init/idt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/idt.c

./o64/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/init.c

./o64/init/ld.o: ./kernel/init/ld.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/ld.c

./o64/init/log.o: ./kernel/init/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/log.c

./o64/init/memory.o: ./kernel/init/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/memory.c

./o64/init/panic.o: ./kernel/init/panic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/panic.c

./o64/init/pg.o: ./kernel/init/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/pg.c

./o64/init/pit.o: ./kernel/init/pit.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/pit.c

./o64/init/print.o: ./kernel/init/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/print.c

./o64/init/rtc.o: ./kernel/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/rtc.c

./o64/init/start.o: ./kernel/init/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/start.c

./o64/init/thrd.o: ./kernel/init/thrd.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/thrd.c

./o64/init/vga.o: ./kernel/init/vga.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/vga.c

./o64/lib/a64/dark.o: ./kernel/lib/a64/dark.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/dark.asm

./o64/lib/a64/spin.o: ./kernel/lib/a64/spin.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/spin.asm

./o64/lib/a64/string.o: ./kernel/lib/a64/string.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/string.asm

./o64/lib/bitarray.o: ./kernel/lib/bitarray.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/bitarray.c

./o64/lib/ctype.o: ./kernel/lib/ctype.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/ctype.c

./o64/lib/huffman.o: ./kernel/lib/huffman.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/huffman.c

./o64/lib/inflate.o: ./kernel/lib/inflate.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/inflate.c

./o64/lib/snprintf.o: ./kernel/lib/snprintf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/snprintf.c

./o64/lib/stdlib.o: ./kernel/lib/stdlib.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/stdlib.c

./o64/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/string.c

./o64/uefi/a64/cpu.o: ./boot/uefi/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/cpu.asm

./o64/uefi/a64/file.o: ./boot/uefi/a64/file.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/file.asm

./o64/uefi/a64/font7x9.o: ./boot/uefi/a64/font7x9.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/font7x9.asm

./o64/uefi/a64/memory.o: ./boot/uefi/a64/memory.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/memory.asm

./o64/uefi/a64/pic.o: ./boot/uefi/a64/pic.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/pic.asm

./o64/uefi/a64/start.o: ./boot/uefi/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/start.asm

./o64/uefi/a64/syscall.o: ./boot/uefi/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/syscall.asm

./o64/uefi/block.o: ./boot/uefi/block.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/block.c

./o64/uefi/file.o: ./boot/uefi/file.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/file.c

./o64/uefi/key.o: ./boot/uefi/key.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/key.c

./o64/uefi/log.o: ./boot/uefi/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/log.c

./o64/uefi/memory.o: ./boot/uefi/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/memory.c

./o64/uefi/misc.o: ./boot/uefi/misc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/misc.c

./o64/uefi/print.o: ./boot/uefi/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/print.c

./o64/uefi/serial.o: ./boot/uefi/serial.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/serial.c

./o64/uefi/syscall.o: ./boot/uefi/syscall.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/syscall.c

./o64/uefi/uefi.o: ./boot/uefi/uefi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/uefi.c

./o64/uefi/video.o: ./boot/uefi/video.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/video.c
