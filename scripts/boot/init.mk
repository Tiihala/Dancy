# Dancy Operating System

##############################################################################

DANCY_START_OBJECTS_32= \
 ./o32/boot/init/a32/start.o \
 ./o32/boot/init/a32/syscall.o \
 ./o32/boot/init/db.o \
 ./o32/boot/init/ld.o \
 ./o32/boot/init/memory.o \
 ./o32/boot/init/print.o \
 ./o32/boot/init/start.o \
 ./o32/common/a32/dark.o \
 ./o32/common/a32/string.o \
 ./o32/common/crc32.o \
 ./o32/common/crc32c.o \
 ./o32/kernel/lib/a32/coff.o \
 ./o32/kernel/lib/a32/spin.o \
 ./o32/kernel/lib/bitarray.o \
 ./o32/kernel/lib/huffman.o \
 ./o32/kernel/lib/inflate.o \
 ./o32/kernel/lib/snprintf.o \
 ./o32/kernel/lib/string.o \

DANCY_START_OBJECTS_64= \
 ./o64/boot/init/a64/start.o \
 ./o64/boot/init/a64/syscall.o \
 ./o64/boot/init/db.o \
 ./o64/boot/init/ld.o \
 ./o64/boot/init/memory.o \
 ./o64/boot/init/print.o \
 ./o64/boot/init/start.o \
 ./o64/common/a64/dark.o \
 ./o64/common/a64/string.o \
 ./o64/common/crc32.o \
 ./o64/common/crc32c.o \
 ./o64/kernel/lib/a64/coff.o \
 ./o64/kernel/lib/a64/spin.o \
 ./o64/kernel/lib/bitarray.o \
 ./o64/kernel/lib/huffman.o \
 ./o64/kernel/lib/inflate.o \
 ./o64/kernel/lib/snprintf.o \
 ./o64/kernel/lib/string.o \

##############################################################################

./system/IN_IA32.AT: $(DANCY_START_OBJECTS_32)
	$(DY_LINK) -o$@ -finit $(DANCY_START_OBJECTS_32)
	$(DY_INIT) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_START_OBJECTS_64)
	$(DY_LINK) -o$@ -finit $(DANCY_START_OBJECTS_64)
	$(DY_INIT) -tx64 --set-header $@

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/boot/init/a32/gdt.o \
 ./o32/boot/init/a32/idt.o \
 ./o32/boot/init/a32/kernel.o \
 ./o32/boot/init/smp/smp.o \
 ./o32/boot/init/smp/start32.o \
 ./o32/boot/init/acpi.o \
 ./o32/boot/init/apic.o \
 ./o32/boot/init/cpu.o \
 ./o32/boot/init/delay.o \
 ./o32/boot/init/fs.o \
 ./o32/boot/init/gui.o \
 ./o32/boot/init/hpet.o \
 ./o32/boot/init/idt.o \
 ./o32/boot/init/init.o \
 ./o32/boot/init/kernel.o \
 ./o32/boot/init/log.o \
 ./o32/boot/init/panic.o \
 ./o32/boot/init/pci.o \
 ./o32/boot/init/pg.o \
 ./o32/boot/init/pit.o \
 ./o32/boot/init/rtc.o \
 ./o32/boot/init/table.o \
 ./o32/boot/init/usb.o \
 ./o32/boot/init/vga.o \
 ./o32/common/fat.o \
 ./o32/common/ttf.o \
 ./o32/kernel/lib/a32/cpu.o \
 ./o32/kernel/lib/stdlib.o \

DANCY_INIT_OBJECTS_64= \
 ./o64/boot/init/a64/gdt.o \
 ./o64/boot/init/a64/idt.o \
 ./o64/boot/init/a64/kernel.o \
 ./o64/boot/init/smp/smp.o \
 ./o64/boot/init/smp/start64.o \
 ./o64/boot/init/acpi.o \
 ./o64/boot/init/apic.o \
 ./o64/boot/init/cpu.o \
 ./o64/boot/init/delay.o \
 ./o64/boot/init/fs.o \
 ./o64/boot/init/gui.o \
 ./o64/boot/init/hpet.o \
 ./o64/boot/init/idt.o \
 ./o64/boot/init/init.o \
 ./o64/boot/init/kernel.o \
 ./o64/boot/init/log.o \
 ./o64/boot/init/panic.o \
 ./o64/boot/init/pci.o \
 ./o64/boot/init/pg.o \
 ./o64/boot/init/pit.o \
 ./o64/boot/init/rtc.o \
 ./o64/boot/init/table.o \
 ./o64/boot/init/usb.o \
 ./o64/boot/init/vga.o \
 ./o64/common/fat.o \
 ./o64/common/ttf.o \
 ./o64/kernel/lib/a64/cpu.o \
 ./o64/kernel/lib/stdlib.o \

##############################################################################

./o32/boot/init.at: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_32)

./o64/boot/init.at: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_64)
