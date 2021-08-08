# Dancy Operating System

##############################################################################

./o32/base/a32/fb.o: ./kernel/base/a32/fb.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/fb.asm

./o32/base/a32/gdt.o: ./kernel/base/a32/gdt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/gdt.asm

./o32/base/a32/idt.o: ./kernel/base/a32/idt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/idt.asm

./o32/base/a32/task.o: ./kernel/base/a32/task.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/task.asm

./o32/base/a32/timer.o: ./kernel/base/a32/timer.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/base/a32/timer.asm

./o32/base/apic.o: ./kernel/base/apic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/apic.c

./o32/base/console.o: ./kernel/base/console.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/console.c

./o32/base/delay.o: ./kernel/base/delay.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/delay.c

./o32/base/fb.o: ./kernel/base/fb.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/fb.c

./o32/base/gdt.o: ./kernel/base/gdt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/gdt.c

./o32/base/heap.o: ./kernel/base/heap.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/heap.c

./o32/base/idt.o: ./kernel/base/idt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/idt.c

./o32/base/irq.o: ./kernel/base/irq.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/irq.c

./o32/base/mm.o: ./kernel/base/mm.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/mm.c

./o32/base/mtx.o: ./kernel/base/mtx.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/mtx.c

./o32/base/panic.o: ./kernel/base/panic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/panic.c

./o32/base/pci.o: ./kernel/base/pci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/pci.c

./o32/base/pg.o: ./kernel/base/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/pg.c

./o32/base/ps2.o: ./kernel/base/ps2.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/ps2.c

./o32/base/runlevel.o: ./kernel/base/runlevel.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/runlevel.c

./o32/base/start.o: ./kernel/base/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/start.c

./o32/base/task.o: ./kernel/base/task.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/task.c

./o32/base/timer.o: ./kernel/base/timer.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/timer.c

./o32/common/crc32.o: ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32.c

./o32/common/crc32c.o: ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/crc32c.c

./o32/common/fat.o: ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/fat.c

./o32/common/ttf.o: ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/ttf.c

./o32/common/utf8.o: ./common/utf8.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./common/utf8.c

./o32/init/a32/gdt.o: ./boot/init/a32/gdt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/gdt.asm

./o32/init/a32/idt.o: ./boot/init/a32/idt.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/idt.asm

./o32/init/a32/kernel.o: ./boot/init/a32/kernel.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/kernel.asm

./o32/init/a32/ld.o: ./boot/init/a32/ld.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/ld.asm

./o32/init/a32/start.o: ./boot/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/start.asm

./o32/init/a32/syscall.o: ./boot/init/a32/syscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./boot/init/a32/syscall.asm

./o32/init/smp/smp.o: ./boot/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/smp.c

./o32/init/smp/start32.o: ./boot/init/smp/start32.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/smp/start32.c

./o32/init/acpi.o: ./boot/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/acpi.c

./o32/init/apic.o: ./boot/init/apic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/apic.c

./o32/init/cpu.o: ./boot/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/cpu.c

./o32/init/db.o: ./boot/init/db.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/db.c

./o32/init/delay.o: ./boot/init/delay.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/delay.c

./o32/init/fs.o: ./boot/init/fs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/fs.c

./o32/init/gui.o: ./boot/init/gui.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/gui.c

./o32/init/hpet.o: ./boot/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/hpet.c

./o32/init/idt.o: ./boot/init/idt.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/idt.c

./o32/init/init.o: ./boot/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/init.c

./o32/init/kernel.o: ./boot/init/kernel.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/kernel.c

./o32/init/ld.o: ./boot/init/ld.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/ld.c

./o32/init/log.o: ./boot/init/log.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/log.c

./o32/init/memory.o: ./boot/init/memory.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/memory.c

./o32/init/mtx.o: ./boot/init/mtx.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/mtx.c

./o32/init/panic.o: ./boot/init/panic.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/panic.c

./o32/init/pci.o: ./boot/init/pci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pci.c

./o32/init/pg.o: ./boot/init/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pg.c

./o32/init/pit.o: ./boot/init/pit.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/pit.c

./o32/init/print.o: ./boot/init/print.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/print.c

./o32/init/rtc.o: ./boot/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/rtc.c

./o32/init/start.o: ./boot/init/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/start.c

./o32/init/table.o: ./boot/init/table.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/table.c

./o32/init/usb.o: ./boot/init/usb.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/usb.c

./o32/init/vga.o: ./boot/init/vga.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./boot/init/vga.c

./o32/lib/a32/cpu.o: ./kernel/lib/a32/cpu.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/cpu.asm

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

./o64/base/a64/fb.o: ./kernel/base/a64/fb.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/fb.asm

./o64/base/a64/gdt.o: ./kernel/base/a64/gdt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/gdt.asm

./o64/base/a64/idt.o: ./kernel/base/a64/idt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/idt.asm

./o64/base/a64/task.o: ./kernel/base/a64/task.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/task.asm

./o64/base/a64/timer.o: ./kernel/base/a64/timer.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/base/a64/timer.asm

./o64/base/apic.o: ./kernel/base/apic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/apic.c

./o64/base/console.o: ./kernel/base/console.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/console.c

./o64/base/delay.o: ./kernel/base/delay.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/delay.c

./o64/base/fb.o: ./kernel/base/fb.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/fb.c

./o64/base/gdt.o: ./kernel/base/gdt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/gdt.c

./o64/base/heap.o: ./kernel/base/heap.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/heap.c

./o64/base/idt.o: ./kernel/base/idt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/idt.c

./o64/base/irq.o: ./kernel/base/irq.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/irq.c

./o64/base/mm.o: ./kernel/base/mm.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/mm.c

./o64/base/mtx.o: ./kernel/base/mtx.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/mtx.c

./o64/base/panic.o: ./kernel/base/panic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/panic.c

./o64/base/pci.o: ./kernel/base/pci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/pci.c

./o64/base/pg.o: ./kernel/base/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/pg.c

./o64/base/ps2.o: ./kernel/base/ps2.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/ps2.c

./o64/base/runlevel.o: ./kernel/base/runlevel.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/runlevel.c

./o64/base/start.o: ./kernel/base/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/start.c

./o64/base/task.o: ./kernel/base/task.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/task.c

./o64/base/timer.o: ./kernel/base/timer.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/timer.c

./o64/common/crc32.o: ./common/crc32.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32.c

./o64/common/crc32c.o: ./common/crc32c.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/crc32c.c

./o64/common/fat.o: ./common/fat.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/fat.c

./o64/common/ttf.o: ./common/ttf.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/ttf.c

./o64/common/utf8.o: ./common/utf8.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./common/utf8.c

./o64/init/a64/gdt.o: ./boot/init/a64/gdt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/gdt.asm

./o64/init/a64/idt.o: ./boot/init/a64/idt.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/idt.asm

./o64/init/a64/kernel.o: ./boot/init/a64/kernel.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/kernel.asm

./o64/init/a64/ld.o: ./boot/init/a64/ld.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/ld.asm

./o64/init/a64/start.o: ./boot/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/start.asm

./o64/init/a64/syscall.o: ./boot/init/a64/syscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./boot/init/a64/syscall.asm

./o64/init/smp/smp.o: ./boot/init/smp/smp.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/smp.c

./o64/init/smp/start64.o: ./boot/init/smp/start64.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/smp/start64.c

./o64/init/acpi.o: ./boot/init/acpi.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/acpi.c

./o64/init/apic.o: ./boot/init/apic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/apic.c

./o64/init/cpu.o: ./boot/init/cpu.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/cpu.c

./o64/init/db.o: ./boot/init/db.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/db.c

./o64/init/delay.o: ./boot/init/delay.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/delay.c

./o64/init/fs.o: ./boot/init/fs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/fs.c

./o64/init/gui.o: ./boot/init/gui.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/gui.c

./o64/init/hpet.o: ./boot/init/hpet.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/hpet.c

./o64/init/idt.o: ./boot/init/idt.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/idt.c

./o64/init/init.o: ./boot/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/init.c

./o64/init/kernel.o: ./boot/init/kernel.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/kernel.c

./o64/init/ld.o: ./boot/init/ld.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/ld.c

./o64/init/log.o: ./boot/init/log.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/log.c

./o64/init/memory.o: ./boot/init/memory.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/memory.c

./o64/init/mtx.o: ./boot/init/mtx.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/mtx.c

./o64/init/panic.o: ./boot/init/panic.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/panic.c

./o64/init/pci.o: ./boot/init/pci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pci.c

./o64/init/pg.o: ./boot/init/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pg.c

./o64/init/pit.o: ./boot/init/pit.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/pit.c

./o64/init/print.o: ./boot/init/print.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/print.c

./o64/init/rtc.o: ./boot/init/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/rtc.c

./o64/init/start.o: ./boot/init/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/start.c

./o64/init/table.o: ./boot/init/table.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/table.c

./o64/init/usb.o: ./boot/init/usb.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/usb.c

./o64/init/vga.o: ./boot/init/vga.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./boot/init/vga.c

./o64/lib/a64/cpu.o: ./kernel/lib/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/cpu.asm

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
