# Dancy Operating System

##############################################################################

LDR512_FILE=-t 2018-01-23T21:49:13 --read-only
LOADER_FILE=-t 2019-03-03T08:58:39 --read-only

##############################################################################

./release/dancy.iso: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t lba --2048 $@ 8192
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)
	$(DY_ISO) -o $@ $@

./release/fdd160.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 160
	$(DY_MCOPY) -i $@ ./system/IN_IA16.AT ::system/IN_IA16.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/fdd720.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 720
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA16.AT ::system/IN_IA16.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/fdd1440.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 1440
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA16.AT ::system/IN_IA16.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/usbhuge.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t chs $@ 31744
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/usbtiny.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t chs $@ 4096
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/dancy.zip: $(DANCY_TARGET_IMAGES) README VERSION
	$(DY_ZIP) -o $@ $(DANCY_TARGET_IMAGES) ./README ./VERSION
