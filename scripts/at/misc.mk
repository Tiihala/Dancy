# Dancy Operating System

##############################################################################

DANCY_MISC_OBJECTS_32= \
 ./o32/misc/rtc.o \
 ./o32/misc/serial.o

DANCY_MISC_OBJECTS_64= \
 ./o64/misc/rtc.o \
 ./o64/misc/serial.o

##############################################################################

./o32/misc.at: $(DANCY_MISC_OBJECTS_32)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_MISC_OBJECTS_32)

./o64/misc.at: $(DANCY_MISC_OBJECTS_64)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_MISC_OBJECTS_64)
