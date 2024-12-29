# Dancy Operating System

##############################################################################

DANCY_MISC_OBJECTS_32= \
 ./o32/kernel/misc/bin.o \
 ./o32/kernel/misc/coff.o \
 ./o32/kernel/misc/console.o \
 ./o32/kernel/misc/dma.o \
 ./o32/kernel/misc/epoch.o \
 ./o32/kernel/misc/fb_user.o \
 ./o32/kernel/misc/floppy.o \
 ./o32/kernel/misc/hdd_fat.o \
 ./o32/kernel/misc/hdd_part.o \
 ./o32/kernel/misc/rtc.o \
 ./o32/kernel/misc/serial.o \
 ./o32/kernel/misc/zero.o \

DANCY_MISC_OBJECTS_64= \
 ./o64/kernel/misc/bin.o \
 ./o64/kernel/misc/coff.o \
 ./o64/kernel/misc/console.o \
 ./o64/kernel/misc/dma.o \
 ./o64/kernel/misc/epoch.o \
 ./o64/kernel/misc/fb_user.o \
 ./o64/kernel/misc/floppy.o \
 ./o64/kernel/misc/hdd_fat.o \
 ./o64/kernel/misc/hdd_part.o \
 ./o64/kernel/misc/rtc.o \
 ./o64/kernel/misc/serial.o \
 ./o64/kernel/misc/zero.o \

##############################################################################

./o32/misc.at: $(DANCY_MISC_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_MISC_OBJECTS_32)

./o64/misc.at: $(DANCY_MISC_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_MISC_OBJECTS_64)
