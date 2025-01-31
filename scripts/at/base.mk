# Dancy Operating System

##############################################################################

DANCY_BASE_OBJECTS_32= \
 ./o32/kernel/base/a32/fb.o \
 ./o32/kernel/base/a32/gdt.o \
 ./o32/kernel/base/a32/idt.o \
 ./o32/kernel/base/a32/task.o \
 ./o32/kernel/base/a32/timer.o \
 ./o32/kernel/base/apic.o \
 ./o32/kernel/base/console.o \
 ./o32/kernel/base/delay.o \
 ./o32/kernel/base/event.o \
 ./o32/kernel/base/fb.o \
 ./o32/kernel/base/gdt.o \
 ./o32/kernel/base/heap.o \
 ./o32/kernel/base/idt.o \
 ./o32/kernel/base/idt_user.o \
 ./o32/kernel/base/irq.o \
 ./o32/kernel/base/kmsg.o \
 ./o32/kernel/base/mm.o \
 ./o32/kernel/base/mtx.o \
 ./o32/kernel/base/panic.o \
 ./o32/kernel/base/pg.o \
 ./o32/kernel/base/ret_user.o \
 ./o32/kernel/base/runlevel.o \
 ./o32/kernel/base/start.o \
 ./o32/kernel/base/task.o \

DANCY_BASE_OBJECTS_64= \
 ./o64/kernel/base/a64/fb.o \
 ./o64/kernel/base/a64/gdt.o \
 ./o64/kernel/base/a64/idt.o \
 ./o64/kernel/base/a64/task.o \
 ./o64/kernel/base/a64/timer.o \
 ./o64/kernel/base/apic.o \
 ./o64/kernel/base/console.o \
 ./o64/kernel/base/delay.o \
 ./o64/kernel/base/event.o \
 ./o64/kernel/base/fb.o \
 ./o64/kernel/base/gdt.o \
 ./o64/kernel/base/heap.o \
 ./o64/kernel/base/idt.o \
 ./o64/kernel/base/idt_user.o \
 ./o64/kernel/base/irq.o \
 ./o64/kernel/base/kmsg.o \
 ./o64/kernel/base/mm.o \
 ./o64/kernel/base/mtx.o \
 ./o64/kernel/base/panic.o \
 ./o64/kernel/base/pg.o \
 ./o64/kernel/base/ret_user.o \
 ./o64/kernel/base/runlevel.o \
 ./o64/kernel/base/start.o \
 ./o64/kernel/base/task.o \

##############################################################################

./o32/base.at: $(DANCY_BASE_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_BASE_OBJECTS_32)

./o64/base.at: $(DANCY_BASE_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_BASE_OBJECTS_64)
