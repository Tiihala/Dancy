# Dancy Operating System

##############################################################################

DANCY_PCI_OBJECTS_32= \
 ./o32/kernel/pci/ahci.o \
 ./o32/kernel/pci/ide_ctrl.o \
 ./o32/kernel/pci/pci.o \

DANCY_PCI_OBJECTS_64= \
 ./o64/kernel/pci/ahci.o \
 ./o64/kernel/pci/ide_ctrl.o \
 ./o64/kernel/pci/pci.o \

##############################################################################

./o32/pci.at: $(DANCY_PCI_OBJECTS_32)
	$(DY_LINK) -o$@ -fat $(DANCY_PCI_OBJECTS_32)

./o64/pci.at: $(DANCY_PCI_OBJECTS_64)
	$(DY_LINK) -o$@ -fat $(DANCY_PCI_OBJECTS_64)
