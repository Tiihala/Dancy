# Dancy Operating System

##############################################################################

DANCY_PS2_OBJECTS_32= \
 ./o32/kernel/ps2/8042.o \
 ./o32/kernel/ps2/keyboard.o \
 ./o32/kernel/ps2/mouse.o \

DANCY_PS2_OBJECTS_64= \
 ./o64/kernel/ps2/8042.o \
 ./o64/kernel/ps2/keyboard.o \
 ./o64/kernel/ps2/mouse.o \

##############################################################################

./o32/ps2.at: $(DANCY_PS2_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_PS2_OBJECTS_32)

./o64/ps2.at: $(DANCY_PS2_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_PS2_OBJECTS_64)
