# Dancy Operating System

##############################################################################

ARCTIC_LIBC_OBJECTS_32= \
 ./arctic/o32/libc/a32/start.o \
 ./arctic/o32/libc/a32/syscall0.o \
 ./arctic/o32/libc/a32/syscall1.o \
 ./arctic/o32/libc/a32/syscall2.o \
 ./arctic/o32/libc/a32/syscall3.o \
 ./arctic/o32/libc/a32/syscall4.o \
 ./arctic/o32/libc/a32/syscall5.o \
 ./arctic/o32/libc/start.o \
 ./o32/_common/a32/dark.o \
 ./o32/_common/a32/string.o \

ARCTIC_LIBC_OBJECTS_64= \
 ./arctic/o64/libc/a64/start.o \
 ./arctic/o64/libc/a64/syscall0.o \
 ./arctic/o64/libc/a64/syscall1.o \
 ./arctic/o64/libc/a64/syscall2.o \
 ./arctic/o64/libc/a64/syscall3.o \
 ./arctic/o64/libc/a64/syscall4.o \
 ./arctic/o64/libc/a64/syscall5.o \
 ./arctic/o64/libc/start.o \
 ./o64/_common/a64/dark.o \
 ./o64/_common/a64/string.o \

##############################################################################

./arctic/o32/libc.a: $(ARCTIC_LIBC_OBJECTS_32)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_32)

./arctic/o64/libc.a: $(ARCTIC_LIBC_OBJECTS_64)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_64)