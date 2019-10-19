# Dancy Operating System

##############################################################################

BOOTX64_EFI_OBJECTS= \
 ./o64/lib/a64/dark.o \
 ./o64/lib/a64/string.o \
 ./o64/lib/ctype.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/stdlib.o \
 ./o64/lib/string.o \
 ./o64/uefi/a64/memory.o \
 ./o64/uefi/a64/start.o \
 ./o64/uefi/log.o \
 ./o64/uefi/memory.o \
 ./o64/uefi/misc.o \
 ./o64/uefi/print.o \
 ./o64/uefi/uefi.o

./efi/boot/BOOTX64.EFI: ./o64/uefi/uefi.at
	$(DY_UEFI) -tx64 -o$@ ./o64/uefi/uefi.at

./o64/uefi/uefi.at: $(BOOTX64_EFI_OBJECTS)
	$(DY_LINK) -fuefi -o$@ $(BOOTX64_EFI_OBJECTS)
