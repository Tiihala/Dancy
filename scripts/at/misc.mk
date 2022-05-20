# Dancy Operating System

##############################################################################

DANCY_MISC_OBJECTS_32= \
 ./o32/misc/dma.o \
 ./o32/misc/epoch.o \
 ./o32/misc/fdn.o \
 ./o32/misc/floppy.o \
 ./o32/misc/rtc.o \
 ./o32/misc/serial.o \
 ./o32/misc/zero.o

DANCY_MISC_OBJECTS_64= \
 ./o64/misc/dma.o \
 ./o64/misc/epoch.o \
 ./o64/misc/fdn.o \
 ./o64/misc/floppy.o \
 ./o64/misc/rtc.o \
 ./o64/misc/serial.o \
 ./o64/misc/zero.o

##############################################################################

./o32/misc.at: $(DANCY_MISC_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_MISC_OBJECTS_32)

./o64/misc.at: $(DANCY_MISC_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_MISC_OBJECTS_64)
