# Dancy Operating System

##############################################################################

BOOTX64_EFI_OBJECTS= \
 ./o64/boot/uefi/a64/cpu.o \
 ./o64/boot/uefi/a64/file.o \
 ./o64/boot/uefi/a64/font7x9.o \
 ./o64/boot/uefi/a64/memory.o \
 ./o64/boot/uefi/a64/pic.o \
 ./o64/boot/uefi/a64/start.o \
 ./o64/boot/uefi/a64/syscall.o \
 ./o64/boot/uefi/block.o \
 ./o64/boot/uefi/file.o \
 ./o64/boot/uefi/key.o \
 ./o64/boot/uefi/log.o \
 ./o64/boot/uefi/memory.o \
 ./o64/boot/uefi/misc.o \
 ./o64/boot/uefi/print.o \
 ./o64/boot/uefi/serial.o \
 ./o64/boot/uefi/syscall.o \
 ./o64/boot/uefi/uefi.o \
 ./o64/boot/uefi/video.o \
 ./o64/common/a64/dark.o \
 ./o64/common/a64/string.o \
 ./o64/common/crc32c.o \
 ./o64/kernel/lib/ctype.o \
 ./o64/kernel/lib/snprintf.o \
 ./o64/kernel/lib/stdlib.o \
 ./o64/kernel/lib/string.o \

##############################################################################

./efi/boot/BOOTX64.EFI: ./o64/boot/uefi.bin
	$(DY_UEFI) -o$@ -tx64 ./o64/boot/uefi.bin

./o64/boot/uefi.bin: $(BOOTX64_EFI_OBJECTS)
	$(DY_LINK) -o$@ -fuefi $(BOOTX64_EFI_OBJECTS)
