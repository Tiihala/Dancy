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
 ./o32/_arctic/libc/ctype/isalnum.o \
 ./o32/_arctic/libc/ctype/isalpha.o \
 ./o32/_arctic/libc/ctype/isblank.o \
 ./o32/_arctic/libc/ctype/iscntrl.o \
 ./o32/_arctic/libc/ctype/isdigit.o \
 ./o32/_arctic/libc/ctype/isgraph.o \
 ./o32/_arctic/libc/ctype/islower.o \
 ./o32/_arctic/libc/ctype/isprint.o \
 ./o32/_arctic/libc/ctype/ispunct.o \
 ./o32/_arctic/libc/ctype/isspace.o \
 ./o32/_arctic/libc/ctype/isupper.o \
 ./o32/_arctic/libc/ctype/isxdigit.o \
 ./o32/_arctic/libc/ctype/tolower.o \
 ./o32/_arctic/libc/ctype/toupper.o \
 ./o32/_arctic/libc/stdio/snprintf.o \
 ./o32/_arctic/libc/stdlib/qsort.o \
 ./o32/_arctic/libc/string/strcat.o \
 ./o32/_arctic/libc/string/strcmp.o \
 ./o32/_arctic/libc/string/strcpy.o \
 ./o32/_arctic/libc/string/strncmp.o \
 ./o32/_arctic/libc/string/strncpy.o \
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
 ./o64/_arctic/libc/ctype/isalnum.o \
 ./o64/_arctic/libc/ctype/isalpha.o \
 ./o64/_arctic/libc/ctype/isblank.o \
 ./o64/_arctic/libc/ctype/iscntrl.o \
 ./o64/_arctic/libc/ctype/isdigit.o \
 ./o64/_arctic/libc/ctype/isgraph.o \
 ./o64/_arctic/libc/ctype/islower.o \
 ./o64/_arctic/libc/ctype/isprint.o \
 ./o64/_arctic/libc/ctype/ispunct.o \
 ./o64/_arctic/libc/ctype/isspace.o \
 ./o64/_arctic/libc/ctype/isupper.o \
 ./o64/_arctic/libc/ctype/isxdigit.o \
 ./o64/_arctic/libc/ctype/tolower.o \
 ./o64/_arctic/libc/ctype/toupper.o \
 ./o64/_arctic/libc/stdio/snprintf.o \
 ./o64/_arctic/libc/stdlib/qsort.o \
 ./o64/_arctic/libc/string/strcat.o \
 ./o64/_arctic/libc/string/strcmp.o \
 ./o64/_arctic/libc/string/strcpy.o \
 ./o64/_arctic/libc/string/strncmp.o \
 ./o64/_arctic/libc/string/strncpy.o \
 ./o64/_arctic/libc/start.o \
 ./o64/_common/a64/dark.o \
 ./o64/_common/a64/string.o \

##############################################################################

./o32/_arctic/libc.a: $(ARCTIC_LIBC_OBJECTS_32)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_32)

./o64/_arctic/libc.a: $(ARCTIC_LIBC_OBJECTS_64)
	$(DY_LINK) -flib -o$@ $(ARCTIC_LIBC_OBJECTS_64)
