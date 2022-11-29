# Dancy Operating System

##############################################################################

ARCTIC_LIBC_OBJECTS_32= \
 ./o32/_arctic/libc/a32/start.o \
 ./o32/_arctic/libc/a32/syscall0.o \
 ./o32/_arctic/libc/a32/syscall1.o \
 ./o32/_arctic/libc/a32/syscall2.o \
 ./o32/_arctic/libc/a32/syscall3.o \
 ./o32/_arctic/libc/a32/syscall4.o \
 ./o32/_arctic/libc/a32/syscall5.o \
 ./o32/_arctic/libc/stdio/snprintf.o \
 ./o32/_arctic/libc/start.o \
 ./o32/_common/a32/dark.o \
 ./o32/_common/a32/string.o \

ARCTIC_LIBC_OBJECTS_64= \
 ./o64/_arctic/libc/a64/start.o \
 ./o64/_arctic/libc/a64/syscall0.o \
 ./o64/_arctic/libc/a64/syscall1.o \
 ./o64/_arctic/libc/a64/syscall2.o \
 ./o64/_arctic/libc/a64/syscall3.o \
 ./o64/_arctic/libc/a64/syscall4.o \
 ./o64/_arctic/libc/a64/syscall5.o \
 ./o64/_arctic/libc/stdio/snprintf.o \
 ./o64/_arctic/libc/start.o \
 ./o64/_common/a64/dark.o \
 ./o64/_common/a64/string.o \

##############################################################################

./o32/_arctic/libc.a: $(ARCTIC_LIBC_OBJECTS_32)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_32)

./o64/_arctic/libc.a: $(ARCTIC_LIBC_OBJECTS_64)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_64)
