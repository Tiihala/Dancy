# Dancy Operating System

##############################################################################

DANCY_PCI_OBJECTS_32= \
 ./o32/pci/ide_ctrl.o \
 ./o32/pci/pci.o

DANCY_PCI_OBJECTS_64= \
 ./o64/pci/ide_ctrl.o \
 ./o64/pci/pci.o

##############################################################################

./o32/pci.at: $(DANCY_PCI_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_PCI_OBJECTS_32)

./o64/pci.at: $(DANCY_PCI_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_PCI_OBJECTS_64)
