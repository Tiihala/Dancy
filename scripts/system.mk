# Dancy Operating System

##############################################################################

DANCY_DEPS=all-tools external o32 o64 system $(DANCY_HEADERS)

o32:
	@mkdir "o32"
	@mkdir "o32/init"
	@mkdir "o32/init/a32"

o64:
	@mkdir "o64"
	@mkdir "o64/init"
	@mkdir "o64/init/a64"

system:
	@mkdir "system"

##############################################################################

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -t loader $@

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/init/a32/bsyscall.o \
 ./o32/init/a32/start.o \
 ./o32/init/init.o

./system/IN_IA32.AT: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -finit $(DANCY_INIT_OBJECTS_32)
	$(DY_INIT) -tia32 --set-header $@

##############################################################################

DANCY_INIT_OBJECTS_64= \
 ./o64/init/a64/bsyscall.o \
 ./o64/init/a64/start.o \
 ./o64/init/init.o

./system/IN_X64.AT: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -finit $(DANCY_INIT_OBJECTS_64)
	$(DY_INIT) -tx64 --set-header $@

##############################################################################

./o32/init/a32/bsyscall.o: ./kernel/init/a32/bsyscall.asm $(DANCY_DEPS)
	$(DANCY_AS) -o$@ -fwin32 ./kernel/init/a32/bsyscall.asm

./o32/init/a32/start.o: ./kernel/init/a32/start.asm $(DANCY_DEPS)
	$(DANCY_AS) -o$@ -fwin32 ./kernel/init/a32/start.asm

./o32/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_OBJECT_32)$@ ./kernel/init/init.c

##############################################################################

./o64/init/a64/bsyscall.o: ./kernel/init/a64/bsyscall.asm $(DANCY_DEPS)
	$(DANCY_AS) -o$@ -fwin64 ./kernel/init/a64/bsyscall.asm

./o64/init/a64/start.o: ./kernel/init/a64/start.asm $(DANCY_DEPS)
	$(DANCY_AS) -o$@ -fwin64 ./kernel/init/a64/start.asm

./o64/init/init.o: ./kernel/init/init.c $(DANCY_DEPS)
	$(DANCY_OBJECT_64)$@ ./kernel/init/init.c
