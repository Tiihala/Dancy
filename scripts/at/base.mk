# Dancy Operating System

##############################################################################

DANCY_BASE_OBJECTS_32= \
 ./o32/base/a32/gdt.o \
 ./o32/base/a32/idt.o \
 ./o32/base/a32/timer.o \
 ./o32/base/apic.o \
 ./o32/base/console.o \
 ./o32/base/delay.o \
 ./o32/base/fb.o \
 ./o32/base/gdt.o \
 ./o32/base/heap.o \
 ./o32/base/idt.o \
 ./o32/base/mtx.o \
 ./o32/base/pg.o \
 ./o32/base/start.o \
 ./o32/base/timer.o

DANCY_BASE_OBJECTS_64= \
 ./o64/base/a64/gdt.o \
 ./o64/base/a64/idt.o \
 ./o64/base/a64/timer.o \
 ./o64/base/apic.o \
 ./o64/base/console.o \
 ./o64/base/delay.o \
 ./o64/base/fb.o \
 ./o64/base/gdt.o \
 ./o64/base/heap.o \
 ./o64/base/idt.o \
 ./o64/base/mtx.o \
 ./o64/base/pg.o \
 ./o64/base/start.o \
 ./o64/base/timer.o

##############################################################################

./o32/base.at: $(DANCY_BASE_OBJECTS_32)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_BASE_OBJECTS_32)

./o64/base.at: $(DANCY_BASE_OBJECTS_64)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_BASE_OBJECTS_64)
