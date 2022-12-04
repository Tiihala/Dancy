# Dancy Operating System

##############################################################################

./o32/_arctic/hello/main.o: \
     ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/hello/main.c

./o32/_arctic/libc/a32/memcmp.o: \
     ./arctic/libc/a32/memcmp.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memcmp.asm

./o32/_arctic/libc/a32/memcpy.o: \
     ./arctic/libc/a32/memcpy.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memcpy.asm

./o32/_arctic/libc/a32/memmove.o: \
     ./arctic/libc/a32/memmove.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memmove.asm

./o32/_arctic/libc/a32/memset.o: \
     ./arctic/libc/a32/memset.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/memset.asm

./o32/_arctic/libc/a32/spin.o: \
     ./arctic/libc/a32/spin.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/spin.asm

./o32/_arctic/libc/a32/start.o: \
     ./arctic/libc/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/start.asm

./o32/_arctic/libc/a32/strlen.o: \
     ./arctic/libc/a32/strlen.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/strlen.asm

./o32/_arctic/libc/a32/syscall0.o: \
     ./arctic/libc/a32/syscall0.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall0.asm

./o32/_arctic/libc/a32/syscall1.o: \
     ./arctic/libc/a32/syscall1.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall1.asm

./o32/_arctic/libc/a32/syscall2.o: \
     ./arctic/libc/a32/syscall2.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall2.asm

./o32/_arctic/libc/a32/syscall3.o: \
     ./arctic/libc/a32/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall3.asm

./o32/_arctic/libc/a32/syscall4.o: \
     ./arctic/libc/a32/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall4.asm

./o32/_arctic/libc/a32/syscall5.o: \
     ./arctic/libc/a32/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall5.asm

./o32/_arctic/libc/ctype/isalnum.o: \
     ./arctic/libc/ctype/isalnum.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isalnum.c

./o32/_arctic/libc/ctype/isalpha.o: \
     ./arctic/libc/ctype/isalpha.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isalpha.c

./o32/_arctic/libc/ctype/isblank.o: \
     ./arctic/libc/ctype/isblank.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isblank.c

./o32/_arctic/libc/ctype/iscntrl.o: \
     ./arctic/libc/ctype/iscntrl.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/iscntrl.c

./o32/_arctic/libc/ctype/isdigit.o: \
     ./arctic/libc/ctype/isdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isdigit.c

./o32/_arctic/libc/ctype/isgraph.o: \
     ./arctic/libc/ctype/isgraph.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isgraph.c

./o32/_arctic/libc/ctype/islower.o: \
     ./arctic/libc/ctype/islower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/islower.c

./o32/_arctic/libc/ctype/isprint.o: \
     ./arctic/libc/ctype/isprint.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isprint.c

./o32/_arctic/libc/ctype/ispunct.o: \
     ./arctic/libc/ctype/ispunct.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/ispunct.c

./o32/_arctic/libc/ctype/isspace.o: \
     ./arctic/libc/ctype/isspace.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isspace.c

./o32/_arctic/libc/ctype/isupper.o: \
     ./arctic/libc/ctype/isupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isupper.c

./o32/_arctic/libc/ctype/isxdigit.o: \
     ./arctic/libc/ctype/isxdigit.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/isxdigit.c

./o32/_arctic/libc/ctype/tolower.o: \
     ./arctic/libc/ctype/tolower.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/tolower.c

./o32/_arctic/libc/ctype/toupper.o: \
     ./arctic/libc/ctype/toupper.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/ctype/toupper.c

./o32/_arctic/libc/stdio/snprintf.o: \
     ./arctic/libc/stdio/snprintf.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdio/snprintf.c

./o32/_arctic/libc/stdlib/qsort.o: \
     ./arctic/libc/stdlib/qsort.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/stdlib/qsort.c

./o32/_arctic/libc/string/strcat.o: \
     ./arctic/libc/string/strcat.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcat.c

./o32/_arctic/libc/string/strcmp.o: \
     ./arctic/libc/string/strcmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcmp.c

./o32/_arctic/libc/string/strcpy.o: \
     ./arctic/libc/string/strcpy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strcpy.c

./o32/_arctic/libc/string/strncmp.o: \
     ./arctic/libc/string/strncmp.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strncmp.c

./o32/_arctic/libc/string/strncpy.o: \
     ./arctic/libc/string/strncpy.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/string/strncpy.c

./o32/_arctic/libc/start.o: \
     ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/start.c

./o64/_arctic/hello/main.o: \
     ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/hello/main.c

./o64/_arctic/libc/a64/memcmp.o: \
     ./arctic/libc/a64/memcmp.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memcmp.asm

./o64/_arctic/libc/a64/memcpy.o: \
     ./arctic/libc/a64/memcpy.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memcpy.asm

./o64/_arctic/libc/a64/memmove.o: \
     ./arctic/libc/a64/memmove.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memmove.asm

./o64/_arctic/libc/a64/memset.o: \
     ./arctic/libc/a64/memset.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/memset.asm

./o64/_arctic/libc/a64/spin.o: \
     ./arctic/libc/a64/spin.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/spin.asm

./o64/_arctic/libc/a64/start.o: \
     ./arctic/libc/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/start.asm

./o64/_arctic/libc/a64/strlen.o: \
     ./arctic/libc/a64/strlen.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/strlen.asm

./o64/_arctic/libc/a64/syscall0.o: \
     ./arctic/libc/a64/syscall0.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall0.asm

./o64/_arctic/libc/a64/syscall1.o: \
     ./arctic/libc/a64/syscall1.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall1.asm

./o64/_arctic/libc/a64/syscall2.o: \
     ./arctic/libc/a64/syscall2.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall2.asm

./o64/_arctic/libc/a64/syscall3.o: \
     ./arctic/libc/a64/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall3.asm

./o64/_arctic/libc/a64/syscall4.o: \
     ./arctic/libc/a64/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall4.asm

./o64/_arctic/libc/a64/syscall5.o: \
     ./arctic/libc/a64/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall5.asm

./o64/_arctic/libc/ctype/isalnum.o: \
     ./arctic/libc/ctype/isalnum.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isalnum.c

./o64/_arctic/libc/ctype/isalpha.o: \
     ./arctic/libc/ctype/isalpha.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isalpha.c

./o64/_arctic/libc/ctype/isblank.o: \
     ./arctic/libc/ctype/isblank.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isblank.c

./o64/_arctic/libc/ctype/iscntrl.o: \
     ./arctic/libc/ctype/iscntrl.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/iscntrl.c

./o64/_arctic/libc/ctype/isdigit.o: \
     ./arctic/libc/ctype/isdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isdigit.c

./o64/_arctic/libc/ctype/isgraph.o: \
     ./arctic/libc/ctype/isgraph.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isgraph.c

./o64/_arctic/libc/ctype/islower.o: \
     ./arctic/libc/ctype/islower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/islower.c

./o64/_arctic/libc/ctype/isprint.o: \
     ./arctic/libc/ctype/isprint.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isprint.c

./o64/_arctic/libc/ctype/ispunct.o: \
     ./arctic/libc/ctype/ispunct.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/ispunct.c

./o64/_arctic/libc/ctype/isspace.o: \
     ./arctic/libc/ctype/isspace.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isspace.c

./o64/_arctic/libc/ctype/isupper.o: \
     ./arctic/libc/ctype/isupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isupper.c

./o64/_arctic/libc/ctype/isxdigit.o: \
     ./arctic/libc/ctype/isxdigit.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/isxdigit.c

./o64/_arctic/libc/ctype/tolower.o: \
     ./arctic/libc/ctype/tolower.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/tolower.c

./o64/_arctic/libc/ctype/toupper.o: \
     ./arctic/libc/ctype/toupper.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/ctype/toupper.c

./o64/_arctic/libc/stdio/snprintf.o: \
     ./arctic/libc/stdio/snprintf.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdio/snprintf.c

./o64/_arctic/libc/stdlib/qsort.o: \
     ./arctic/libc/stdlib/qsort.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/stdlib/qsort.c

./o64/_arctic/libc/string/strcat.o: \
     ./arctic/libc/string/strcat.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcat.c

./o64/_arctic/libc/string/strcmp.o: \
     ./arctic/libc/string/strcmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcmp.c

./o64/_arctic/libc/string/strcpy.o: \
     ./arctic/libc/string/strcpy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strcpy.c

./o64/_arctic/libc/string/strncmp.o: \
     ./arctic/libc/string/strncmp.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strncmp.c

./o64/_arctic/libc/string/strncpy.o: \
     ./arctic/libc/string/strncpy.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/string/strncpy.c

./o64/_arctic/libc/start.o: \
     ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/start.c
