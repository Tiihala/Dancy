# Dancy Operating System

##############################################################################

./release/dancy.iso: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t lba --2048 $@ 8192
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT
	$(DY_ISO) -o $@ $@

./release/fdd720.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 720
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT

./release/fdd1440.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 1440
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT
