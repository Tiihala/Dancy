# Dancy Operating System

##############################################################################

./arctic/o32/hello/main.o: ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/hello/main.c

./arctic/o32/libc/a32/start.o: ./arctic/libc/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/start.asm

./arctic/o32/libc/a32/syscall3.o: ./arctic/libc/a32/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall3.asm

./arctic/o32/libc/a32/syscall4.o: ./arctic/libc/a32/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall4.asm

./arctic/o32/libc/a32/syscall5.o: ./arctic/libc/a32/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./arctic/libc/a32/syscall5.asm

./arctic/o32/libc/start.o: ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O32)$@ ./arctic/libc/start.c

./arctic/o64/hello/main.o: ./arctic/hello/main.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/hello/main.c

./arctic/o64/libc/a64/start.o: ./arctic/libc/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/start.asm

./arctic/o64/libc/a64/syscall3.o: ./arctic/libc/a64/syscall3.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall3.asm

./arctic/o64/libc/a64/syscall4.o: ./arctic/libc/a64/syscall4.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall4.asm

./arctic/o64/libc/a64/syscall5.o: ./arctic/libc/a64/syscall5.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./arctic/libc/a64/syscall5.asm

./arctic/o64/libc/start.o: ./arctic/libc/start.c $(DANCY_DEPS)
	$(ARCTIC_O64)$@ ./arctic/libc/start.c
