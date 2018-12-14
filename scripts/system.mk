# Dancy Operating System

##############################################################################

./external/external.sh:
	@bash ./scripts/external.sh

##############################################################################

DANCY_INIT_OBJECTS_32= \
 ./o32/init/a32/bsyscall.o \
 ./o32/init/a32/start.o \
 ./o32/init/init.o \
 ./o32/lib/a32/string.o

DANCY_INIT_OBJECTS_64= \
 ./o64/init/a64/bsyscall.o \
 ./o64/init/a64/start.o \
 ./o64/init/init.o \
 ./o64/lib/a64/string.o

./system/IN_IA32.AT: $(DANCY_INIT_OBJECTS_32)
	$(DY_LINK) -o$@ -finit $(DANCY_INIT_OBJECTS_32)
	$(DY_INIT) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_INIT_OBJECTS_64)
	$(DY_LINK) -o$@ -finit $(DANCY_INIT_OBJECTS_64)
	$(DY_INIT) -tx64 --set-header $@

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tloader $@
