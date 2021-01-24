# Dancy Operating System

##############################################################################

BOOTX64_EFI_OBJECTS= \
 ./o64/common/crc32c.o \
 ./o64/lib/a64/dark.o \
 ./o64/lib/a64/string.o \
 ./o64/lib/ctype.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/stdlib.o \
 ./o64/lib/string.o \
 ./o64/uefi/a64/cpu.o \
 ./o64/uefi/a64/file.o \
 ./o64/uefi/a64/font7x9.o \
 ./o64/uefi/a64/memory.o \
 ./o64/uefi/a64/pic.o \
 ./o64/uefi/a64/start.o \
 ./o64/uefi/a64/syscall.o \
 ./o64/uefi/block.o \
 ./o64/uefi/file.o \
 ./o64/uefi/key.o \
 ./o64/uefi/log.o \
 ./o64/uefi/memory.o \
 ./o64/uefi/misc.o \
 ./o64/uefi/print.o \
 ./o64/uefi/serial.o \
 ./o64/uefi/syscall.o \
 ./o64/uefi/uefi.o \
 ./o64/uefi/video.o

##############################################################################

./efi/boot/BOOTX64.EFI: ./o64/uefi.at
	$(DY_UEFI) -tx64 -o$@ ./o64/uefi.at

./o64/uefi.at: $(BOOTX64_EFI_OBJECTS)
	$(DY_LINK) -fuefi -o$@ $(BOOTX64_EFI_OBJECTS)
