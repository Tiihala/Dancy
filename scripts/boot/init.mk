# Dancy Operating System

##############################################################################

DANCY_START_OBJECTS_32= \
 ./o32/_boot/init/a32/start.o \
 ./o32/_boot/init/a32/syscall.o \
 ./o32/_boot/init/db.o \
 ./o32/_boot/init/ld.o \
 ./o32/_boot/init/memory.o \
 ./o32/_boot/init/print.o \
 ./o32/_boot/init/start.o \
 ./o32/_common/a32/dark.o \
 ./o32/_common/a32/string.o \
 ./o32/_common/crc32.o \
 ./o32/_common/crc32c.o \
 ./o32/lib/a32/coff.o \
 ./o32/lib/a32/spin.o \
 ./o32/lib/bitarray.o \
 ./o32/lib/huffman.o \
 ./o32/lib/inflate.o \
 ./o32/lib/snprintf.o \
 ./o32/lib/string.o \

DANCY_START_OBJECTS_64= \
 ./o64/_boot/init/a64/start.o \
 ./o64/_boot/init/a64/syscall.o \
 ./o64/_boot/init/db.o \
 ./o64/_boot/init/ld.o \
 ./o64/_boot/init/memory.o \
 ./o64/_boot/init/print.o \
 ./o64/_boot/init/start.o \
 ./o64/_common/a64/dark.o \
 ./o64/_common/a64/string.o \
 ./o64/_common/crc32.o \
 ./o64/_common/crc32c.o \
 ./o64/lib/a64/coff.o \
 ./o64/lib/a64/spin.o \
 ./o64/lib/bitarray.o \
 ./o64/lib/huffman.o \
 ./o64/lib/inflate.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/string.o \

##############################################################################

./system/IN_IA32.AT: $(DANCY_START_OBJECTS_32)
	$(DY_LINK) -o$@ -finit $(DANCY_START_OBJECTS_32)
	$(DY_INIT) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_START_OBJECTS_64)
	$(DY_LINK) -o$@ -finit $(DANCY_START_OBJECTS_64)
	$(DY_INIT) -tx64 --set-header $@

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/_boot/init/a32/gdt.o \
 ./o32/_boot/init/a32/idt.o \
 ./o32/_boot/init/a32/kernel.o \
 ./o32/_boot/init/smp/smp.o \
 ./o32/_boot/init/smp/start32.o \
 ./o32/_boot/init/acpi.o \
 ./o32/_boot/init/apic.o \
 ./o32/_boot/init/cpu.o \
 ./o32/_boot/init/delay.o \
 ./o32/_boot/init/fs.o \
 ./o32/_boot/init/gui.o \
 ./o32/_boot/init/hpet.o \
 ./o32/_boot/init/idt.o \
 ./o32/_boot/init/init.o \
 ./o32/_boot/init/kernel.o \
 ./o32/_boot/init/log.o \
 ./o32/_boot/init/panic.o \
 ./o32/_boot/init/pci.o \
 ./o32/_boot/init/pg.o \
 ./o32/_boot/init/pit.o \
 ./o32/_boot/init/rtc.o \
 ./o32/_boot/init/table.o \
 ./o32/_boot/init/usb.o \
 ./o32/_boot/init/vga.o \
 ./o32/_common/fat.o \
 ./o32/_common/ttf.o \
 ./o32/lib/a32/cpu.o \
 ./o32/lib/stdlib.o \

DANCY_INIT_OBJECTS_64= \
 ./o64/_boot/init/a64/gdt.o \
 ./o64/_boot/init/a64/idt.o \
 ./o64/_boot/init/a64/kernel.o \
 ./o64/_boot/init/smp/smp.o \
 ./o64/_boot/init/smp/start64.o \
 ./o64/_boot/init/acpi.o \
 ./o64/_boot/init/apic.o \
 ./o64/_boot/init/cpu.o \
 ./o64/_boot/init/delay.o \
 ./o64/_boot/init/fs.o \
 ./o64/_boot/init/gui.o \
 ./o64/_boot/init/hpet.o \
 ./o64/_boot/init/idt.o \
 ./o64/_boot/init/init.o \
 ./o64/_boot/init/kernel.o \
 ./o64/_boot/init/log.o \
 ./o64/_boot/init/panic.o \
 ./o64/_boot/init/pci.o \
 ./o64/_boot/init/pg.o \
 ./o64/_boot/init/pit.o \
 ./o64/_boot/init/rtc.o \
 ./o64/_boot/init/table.o \
 ./o64/_boot/init/usb.o \
 ./o64/_boot/init/vga.o \
 ./o64/_common/fat.o \
 ./o64/_common/ttf.o \
 ./o64/lib/a64/cpu.o \
 ./o64/lib/stdlib.o \

##############################################################################

./o32/_boot/init.at: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_32)

./o64/_boot/init.at: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_INIT_OBJECTS_64)
