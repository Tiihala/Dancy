# Dancy Operating System

##############################################################################

DANCY_LEGACY_ASM= \
 ./legacy/in_ia16.asm \
 ./legacy/inc/int20h.asm

./system/IN_IA16.AT: $(DANCY_LEGACY_ASM) ./bin/dy-init$(DANCY_EXE)
	$(DANCY_AS) -I./legacy/ -fbin -o$@ ./legacy/in_ia16.asm
	$(DY_INIT) -tia16 --set-header $@
