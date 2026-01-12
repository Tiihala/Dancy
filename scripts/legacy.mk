# Dancy Operating System

##############################################################################

DANCY_LEGACY_ASM= \
 ./boot/in_ia16/in_ia16.asm \
 ./boot/in_ia16/inc/int20h.asm \

./system/IN_IA16.AT: $(DANCY_DEPS) $(DANCY_LEGACY_ASM)
	$(DANCY_AS) -I./boot/in_ia16/ -fbin -o$@ ./boot/in_ia16/in_ia16.asm
	$(DY_INIT) -tia16 --set-header $@
