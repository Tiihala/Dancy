# Dancy Operating System

##############################################################################

ACPIOS_OBJECTS_32= \
 ./o32/acpios/a32/glblock.o \
 ./o32/acpios/osdancy.o

ACPIOS_OBJECTS_64= \
 ./o64/acpios/a64/glblock.o \
 ./o64/acpios/osdancy.o

##############################################################################

./o32/acpios.at: $(ACPIOS_OBJECTS_32) ./bin/dy-link$(DANCY_EXE)
	$(DY_LINK) -o$@ -fat $(ACPIOS_OBJECTS_32)

./o64/acpios.at: $(ACPIOS_OBJECTS_64) ./bin/dy-link$(DANCY_EXE)
	$(DY_LINK) -o$@ -fat $(ACPIOS_OBJECTS_64)

##############################################################################
