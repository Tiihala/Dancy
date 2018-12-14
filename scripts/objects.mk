# Dancy Operating System

##############################################################################

./o32/init/a32/bsyscall.o: ./kernel/init/a32/bsyscall.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/bsyscall.asm

./o32/init/a32/start.o: ./kernel/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/init/a32/start.asm

./o32/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O32)$@ ./kernel/init/init.c

./o32/lib/a32/string.o: ./kernel/lib/a32/string.asm $(DANCY_DEPS)
	$(DANCY_A32)$@ ./kernel/lib/a32/string.asm

./o64/init/a64/bsyscall.o: ./kernel/init/a64/bsyscall.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/bsyscall.asm

./o64/init/a64/start.o: ./kernel/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/init/a64/start.asm

./o64/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_O64)$@ ./kernel/init/init.c

./o64/lib/a64/string.o: ./kernel/lib/a64/string.asm $(DANCY_DEPS)
	$(DANCY_A64)$@ ./kernel/lib/a64/string.asm
