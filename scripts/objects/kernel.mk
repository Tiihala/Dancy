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

./o32/base/event.o: ./kernel/base/event.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/event.c

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

./o32/base/pg.o: ./kernel/base/pg.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/pg.c

./o32/base/runlevel.o: ./kernel/base/runlevel.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/runlevel.c

./o32/base/start.o: ./kernel/base/start.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/start.c

./o32/base/task.o: ./kernel/base/task.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/task.c

./o32/base/timer.o: ./kernel/base/timer.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/base/timer.c

./o32/debug/debug.o: ./kernel/debug/debug.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/debug/debug.c

./o32/lib/a32/coff.o: ./kernel/lib/a32/coff.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/coff.asm

./o32/lib/a32/cpu.o: ./kernel/lib/a32/cpu.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/cpu.asm

./o32/lib/a32/spin.o: ./kernel/lib/a32/spin.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/spin.asm

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

./o32/lib/strerror.o: ./kernel/lib/strerror.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/strerror.c

./o32/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/lib/string.c

./o32/misc/bin.o: ./kernel/misc/bin.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/bin.c

./o32/misc/dma.o: ./kernel/misc/dma.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/dma.c

./o32/misc/epoch.o: ./kernel/misc/epoch.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/epoch.c

./o32/misc/floppy.o: ./kernel/misc/floppy.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/floppy.c

./o32/misc/hdd_fat.o: ./kernel/misc/hdd_fat.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/hdd_fat.c

./o32/misc/hdd_part.o: ./kernel/misc/hdd_part.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/hdd_part.c

./o32/misc/rtc.o: ./kernel/misc/rtc.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/rtc.c

./o32/misc/serial.o: ./kernel/misc/serial.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/serial.c

./o32/misc/zero.o: ./kernel/misc/zero.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/misc/zero.c

./o32/pci/ide_ctrl.o: ./kernel/pci/ide_ctrl.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/pci/ide_ctrl.c

./o32/pci/pci.o: ./kernel/pci/pci.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/pci/pci.c

./o32/ps2/8042.o: ./kernel/ps2/8042.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/ps2/8042.c

./o32/ps2/keyboard.o: ./kernel/ps2/keyboard.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/ps2/keyboard.c

./o32/ps2/mouse.o: ./kernel/ps2/mouse.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/ps2/mouse.c

./o32/sched/sched.o: ./kernel/sched/sched.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/sched/sched.c

./o32/syscall/syscall.o: ./kernel/syscall/syscall.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/syscall/syscall.c

./o32/vfs/default.o: ./kernel/vfs/default.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/default.c

./o32/vfs/fat_io.o: ./kernel/vfs/fat_io.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/fat_io.c

./o32/vfs/path.o: ./kernel/vfs/path.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/path.c

./o32/vfs/root.o: ./kernel/vfs/root.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/root.c

./o32/vfs/vfs.o: ./kernel/vfs/vfs.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/vfs/vfs.c

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

./o64/base/event.o: ./kernel/base/event.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/event.c

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

./o64/base/pg.o: ./kernel/base/pg.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/pg.c

./o64/base/runlevel.o: ./kernel/base/runlevel.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/runlevel.c

./o64/base/start.o: ./kernel/base/start.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/start.c

./o64/base/task.o: ./kernel/base/task.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/task.c

./o64/base/timer.o: ./kernel/base/timer.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/base/timer.c

./o64/debug/debug.o: ./kernel/debug/debug.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/debug/debug.c

./o64/lib/a64/coff.o: ./kernel/lib/a64/coff.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/coff.asm

./o64/lib/a64/cpu.o: ./kernel/lib/a64/cpu.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/cpu.asm

./o64/lib/a64/spin.o: ./kernel/lib/a64/spin.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/spin.asm

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

./o64/lib/strerror.o: ./kernel/lib/strerror.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/strerror.c

./o64/lib/string.o: ./kernel/lib/string.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/lib/string.c

./o64/misc/bin.o: ./kernel/misc/bin.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/bin.c

./o64/misc/dma.o: ./kernel/misc/dma.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/dma.c

./o64/misc/epoch.o: ./kernel/misc/epoch.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/epoch.c

./o64/misc/floppy.o: ./kernel/misc/floppy.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/floppy.c

./o64/misc/hdd_fat.o: ./kernel/misc/hdd_fat.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/hdd_fat.c

./o64/misc/hdd_part.o: ./kernel/misc/hdd_part.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/hdd_part.c

./o64/misc/rtc.o: ./kernel/misc/rtc.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/rtc.c

./o64/misc/serial.o: ./kernel/misc/serial.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/serial.c

./o64/misc/zero.o: ./kernel/misc/zero.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/misc/zero.c

./o64/pci/ide_ctrl.o: ./kernel/pci/ide_ctrl.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/pci/ide_ctrl.c

./o64/pci/pci.o: ./kernel/pci/pci.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/pci/pci.c

./o64/ps2/8042.o: ./kernel/ps2/8042.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/ps2/8042.c

./o64/ps2/keyboard.o: ./kernel/ps2/keyboard.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/ps2/keyboard.c

./o64/ps2/mouse.o: ./kernel/ps2/mouse.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/ps2/mouse.c

./o64/sched/sched.o: ./kernel/sched/sched.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/sched/sched.c

./o64/syscall/syscall.o: ./kernel/syscall/syscall.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/syscall/syscall.c

./o64/vfs/default.o: ./kernel/vfs/default.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/default.c

./o64/vfs/fat_io.o: ./kernel/vfs/fat_io.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/fat_io.c

./o64/vfs/path.o: ./kernel/vfs/path.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/path.c

./o64/vfs/root.o: ./kernel/vfs/root.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/root.c

./o64/vfs/vfs.o: ./kernel/vfs/vfs.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/vfs/vfs.c
