# Dancy Operating System

##############################################################################

BOOTX64_EFI_OBJECTS= \
 ./o64/_boot/uefi/a64/cpu.o \
 ./o64/_boot/uefi/a64/file.o \
 ./o64/_boot/uefi/a64/font7x9.o \
 ./o64/_boot/uefi/a64/memory.o \
 ./o64/_boot/uefi/a64/pic.o \
 ./o64/_boot/uefi/a64/start.o \
 ./o64/_boot/uefi/a64/syscall.o \
 ./o64/_boot/uefi/block.o \
 ./o64/_boot/uefi/file.o \
 ./o64/_boot/uefi/key.o \
 ./o64/_boot/uefi/log.o \
 ./o64/_boot/uefi/memory.o \
 ./o64/_boot/uefi/misc.o \
 ./o64/_boot/uefi/print.o \
 ./o64/_boot/uefi/serial.o \
 ./o64/_boot/uefi/syscall.o \
 ./o64/_boot/uefi/uefi.o \
 ./o64/_boot/uefi/video.o \
 ./o64/_common/a64/dark.o \
 ./o64/_common/a64/string.o \
 ./o64/_common/crc32c.o \
 ./o64/lib/ctype.o \
 ./o64/lib/snprintf.o \
 ./o64/lib/stdlib.o \
 ./o64/lib/string.o \

##############################################################################

./efi/boot/BOOTX64.EFI: ./o64/_boot/uefi.bin
	$(DY_UEFI) -o$@ -tx64 ./o64/_boot/uefi.bin

./o64/_boot/uefi.bin: $(BOOTX64_EFI_OBJECTS)
	$(DY_LINK) -o$@ -fuefi $(BOOTX64_EFI_OBJECTS)
