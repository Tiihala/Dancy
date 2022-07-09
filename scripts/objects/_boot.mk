# Dancy Operating System

##############################################################################

./o32/_boot/init/a32/gdt.o: ./boot/init/a32/gdt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/gdt.asm

./o32/_boot/init/a32/idt.o: ./boot/init/a32/idt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/idt.asm

./o32/_boot/init/a32/kernel.o: ./boot/init/a32/kernel.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/kernel.asm

./o32/_boot/init/a32/start.o: ./boot/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/start.asm

./o32/_boot/init/a32/syscall.o: ./boot/init/a32/syscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/syscall.asm

./o32/_boot/init/smp/smp.o: ./boot/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/smp.c

./o32/_boot/init/smp/start32.o: ./boot/init/smp/start32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/start32.c

./o32/_boot/init/acpi.o: ./boot/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/acpi.c

./o32/_boot/init/apic.o: ./boot/init/apic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/apic.c

./o32/_boot/init/cpu.o: ./boot/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/cpu.c

./o32/_boot/init/db.o: ./boot/init/db.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/db.c

./o32/_boot/init/delay.o: ./boot/init/delay.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/delay.c

./o32/_boot/init/fs.o: ./boot/init/fs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/fs.c

./o32/_boot/init/gui.o: ./boot/init/gui.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/gui.c

./o32/_boot/init/hpet.o: ./boot/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/hpet.c

./o32/_boot/init/idt.o: ./boot/init/idt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/idt.c

./o32/_boot/init/init.o: ./boot/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/init.c

./o32/_boot/init/kernel.o: ./boot/init/kernel.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/kernel.c

./o32/_boot/init/ld.o: ./boot/init/ld.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/ld.c

./o32/_boot/init/log.o: ./boot/init/log.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/log.c

./o32/_boot/init/memory.o: ./boot/init/memory.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/memory.c

./o32/_boot/init/panic.o: ./boot/init/panic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/panic.c

./o32/_boot/init/pci.o: ./boot/init/pci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pci.c

./o32/_boot/init/pg.o: ./boot/init/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pg.c

./o32/_boot/init/pit.o: ./boot/init/pit.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pit.c

./o32/_boot/init/print.o: ./boot/init/print.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/print.c

./o32/_boot/init/rtc.o: ./boot/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/rtc.c

./o32/_boot/init/start.o: ./boot/init/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/start.c

./o32/_boot/init/table.o: ./boot/init/table.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/table.c

./o32/_boot/init/usb.o: ./boot/init/usb.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/usb.c

./o32/_boot/init/vga.o: ./boot/init/vga.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/vga.c

./o64/_boot/init/a64/gdt.o: ./boot/init/a64/gdt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/gdt.asm

./o64/_boot/init/a64/idt.o: ./boot/init/a64/idt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/idt.asm

./o64/_boot/init/a64/kernel.o: ./boot/init/a64/kernel.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/kernel.asm

./o64/_boot/init/a64/start.o: ./boot/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/start.asm

./o64/_boot/init/a64/syscall.o: ./boot/init/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/syscall.asm

./o64/_boot/init/smp/smp.o: ./boot/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/smp.c

./o64/_boot/init/smp/start64.o: ./boot/init/smp/start64.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/start64.c

./o64/_boot/init/acpi.o: ./boot/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/acpi.c

./o64/_boot/init/apic.o: ./boot/init/apic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/apic.c

./o64/_boot/init/cpu.o: ./boot/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/cpu.c

./o64/_boot/init/db.o: ./boot/init/db.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/db.c

./o64/_boot/init/delay.o: ./boot/init/delay.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/delay.c

./o64/_boot/init/fs.o: ./boot/init/fs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/fs.c

./o64/_boot/init/gui.o: ./boot/init/gui.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/gui.c

./o64/_boot/init/hpet.o: ./boot/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/hpet.c

./o64/_boot/init/idt.o: ./boot/init/idt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/idt.c

./o64/_boot/init/init.o: ./boot/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/init.c

./o64/_boot/init/kernel.o: ./boot/init/kernel.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/kernel.c

./o64/_boot/init/ld.o: ./boot/init/ld.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/ld.c

./o64/_boot/init/log.o: ./boot/init/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/log.c

./o64/_boot/init/memory.o: ./boot/init/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/memory.c

./o64/_boot/init/panic.o: ./boot/init/panic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/panic.c

./o64/_boot/init/pci.o: ./boot/init/pci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pci.c

./o64/_boot/init/pg.o: ./boot/init/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pg.c

./o64/_boot/init/pit.o: ./boot/init/pit.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pit.c

./o64/_boot/init/print.o: ./boot/init/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/print.c

./o64/_boot/init/rtc.o: ./boot/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/rtc.c

./o64/_boot/init/start.o: ./boot/init/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/start.c

./o64/_boot/init/table.o: ./boot/init/table.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/table.c

./o64/_boot/init/usb.o: ./boot/init/usb.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/usb.c

./o64/_boot/init/vga.o: ./boot/init/vga.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/vga.c

./o64/_boot/uefi/a64/cpu.o: ./boot/uefi/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/cpu.asm

./o64/_boot/uefi/a64/file.o: ./boot/uefi/a64/file.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/file.asm

./o64/_boot/uefi/a64/font7x9.o: ./boot/uefi/a64/font7x9.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/font7x9.asm

./o64/_boot/uefi/a64/memory.o: ./boot/uefi/a64/memory.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/memory.asm

./o64/_boot/uefi/a64/pic.o: ./boot/uefi/a64/pic.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/pic.asm

./o64/_boot/uefi/a64/start.o: ./boot/uefi/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/start.asm

./o64/_boot/uefi/a64/syscall.o: ./boot/uefi/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/uefi/a64/syscall.asm

./o64/_boot/uefi/block.o: ./boot/uefi/block.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/block.c

./o64/_boot/uefi/file.o: ./boot/uefi/file.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/file.c

./o64/_boot/uefi/key.o: ./boot/uefi/key.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/key.c

./o64/_boot/uefi/log.o: ./boot/uefi/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/log.c

./o64/_boot/uefi/memory.o: ./boot/uefi/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/memory.c

./o64/_boot/uefi/misc.o: ./boot/uefi/misc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/misc.c

./o64/_boot/uefi/print.o: ./boot/uefi/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/print.c

./o64/_boot/uefi/serial.o: ./boot/uefi/serial.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/serial.c

./o64/_boot/uefi/syscall.o: ./boot/uefi/syscall.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/syscall.c

./o64/_boot/uefi/uefi.o: ./boot/uefi/uefi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/uefi.c

./o64/_boot/uefi/video.o: ./boot/uefi/video.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/uefi/video.c
