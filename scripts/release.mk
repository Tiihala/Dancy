# Dancy Operating System

##############################################################################

LDR512_FILE=-t 2018-01-23T21:49:13 --read-only
LOADER_FILE=-t 2019-03-03T08:58:39 --read-only

##############################################################################

./release/dancy.iso: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t lba --2048 $@ 8192
	$(DY_MCOPY) -i $@ ./efi/boot/BOOTX64.EFI ::efi/boot/BOOTX64.EFI
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/DB_000.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_100.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_200.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_300.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_400.AT --db
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)
	$(DY_ISO) --uefi -o $@ $@

./release/fdd160.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 160 --fixed-dl
	$(DY_MCOPY) -i $@ ./system/IN_IA16.AT ::system/IN_IA16.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/fdd720a.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 720 --fixed-dl
	$(DY_MCOPY) -i $@ ./system/DB_000.AT 8 --db
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/fdd720b.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 720 --fixed-dl
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/DB_008.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_100.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_300.AT --db
	$(DY_MCOPY) -i $@ ./system/IN_IA16.AT ::system/IN_IA16.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/fdd1440.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t floppy $@ 1440
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/DB_000.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_100.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_300.AT --db
	$(DY_MCOPY) -i $@ ./system/IN_IA16.AT ::system/IN_IA16.AT
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

./release/usbtiny.img: $(DANCY_TARGET_SYSTEM)
	$(DY_VBR) -t chs $@ 4096
	$(DY_MCOPY) -i $@ ./efi/boot/BOOTX64.EFI ::efi/boot/BOOTX64.EFI
	$(DY_MCOPY) -i $@ ./system/CONFIG.AT ::system/CONFIG.AT
	$(DY_MCOPY) -i $@ ./system/DB_000.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_100.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_200.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_300.AT --db
	$(DY_MCOPY) -i $@ ./system/DB_400.AT --db
	$(DY_MCOPY) -i $@ ./system/IN_IA32.AT ::system/IN_IA32.AT
	$(DY_MCOPY) -i $@ ./system/IN_X64.AT ::system/IN_X64.AT
	$(DY_MCOPY) -i $@ ./LOADER.512 ::LOADER.512 $(LDR512_FILE)
	$(DY_MCOPY) -i $@ ./LOADER.AT ::LOADER.AT $(LOADER_FILE)

##############################################################################

DANCY_ZIP_FILES= \
 ./README \
 ./release/dancy.iso \
 ./release/fdd1440.img \
 ./release/usbtiny.img \

./release/dancy.zip: $(DANCY_TARGET_IMAGES) ./README
	$(DY_ZIP) -o $@ --single-dir $(DANCY_ZIP_FILES)
