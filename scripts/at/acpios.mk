# Dancy Operating System

##############################################################################

DANCY_ACPIOS_OBJECTS_32= \
 ./o32/acpios/a32/glblock.o

DANCY_ACPIOS_OBJECTS_64= \
 ./o64/acpios/a64/glblock.o

##############################################################################

./o32/acpios.at: $(DANCY_ACPIOS_OBJECTS_32)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_ACPIOS_OBJECTS_32)

./o64/acpios.at: $(DANCY_ACPIOS_OBJECTS_64)
	$(DY_LINK) -o$@ -fat --export-all $(DANCY_ACPIOS_OBJECTS_64)
