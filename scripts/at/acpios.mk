# Dancy Operating System

##############################################################################

ACPIOS_OBJECTS_32= \
 ./o32/kernel/acpios/a32/glblock.o \
 ./o32/kernel/acpios/osdancy.o \

ACPIOS_OBJECTS_64= \
 ./o64/kernel/acpios/a64/glblock.o \
 ./o64/kernel/acpios/osdancy.o \

##############################################################################

./o32/acpios.at: $(ACPIOS_OBJECTS_32) ./bin/dy-link$(DANCY_EXE)
	$(DY_LINK) -o$@ -fat $(ACPIOS_OBJECTS_32)

./o64/acpios.at: $(ACPIOS_OBJECTS_64) ./bin/dy-link$(DANCY_EXE)
	$(DY_LINK) -o$@ -fat $(ACPIOS_OBJECTS_64)

##############################################################################

./o32/kernel/acpios/a32/glblock.o: \
    ./kernel/acpios/a32/glblock.asm $(ACPICA_DEPS)
	$(DANCY_A32)$@ ./kernel/acpios/a32/glblock.asm

./o32/kernel/acpios/osdancy.o: \
    ./kernel/acpios/osdancy.c $(ACPICA_DEPS)
	$(ACPICA_O32)$@ ./kernel/acpios/osdancy.c

./o64/kernel/acpios/a64/glblock.o: \
    ./kernel/acpios/a64/glblock.asm $(ACPICA_DEPS)
	$(DANCY_A64)$@ ./kernel/acpios/a64/glblock.asm

./o64/kernel/acpios/osdancy.o: \
    ./kernel/acpios/osdancy.c $(ACPICA_DEPS)
	$(ACPICA_O64)$@ ./kernel/acpios/osdancy.c
