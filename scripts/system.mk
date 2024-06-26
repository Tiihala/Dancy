# Dancy Operating System

##############################################################################

DANCY_DB_000_FILES= \
 ./o32/boot/init.at \
 ./o64/boot/init.at \
 ./arctic/root.img \
 ./share/fonts/dcysan.ttf \
 ./share/fonts/dcysanb.ttf \
 ./share/fonts/dcysanm.ttf \

DANCY_DB_100_FILES= \
 ./o32/lib.at \
 ./o32/base.at \
 ./o32/debug.at \
 ./o32/misc.at \
 ./o32/pci.at \
 ./o32/ps2.at \
 ./o32/run.at \
 ./o32/sched.at \
 ./o32/syscall.at \
 ./o32/vfs.at \
 $(ACPICA_O32_AT)

DANCY_DB_200_FILES= \
 ./o64/lib.at \
 ./o64/base.at \
 ./o64/debug.at \
 ./o64/misc.at \
 ./o64/pci.at \
 ./o64/ps2.at \
 ./o64/run.at \
 ./o64/sched.at \
 ./o64/syscall.at \
 ./o64/vfs.at \
 $(ACPICA_O64_AT)

DANCY_DB_300_FILES= \
 ./arctic/bin32.img \

DANCY_DB_400_FILES= \
 ./arctic/bin64.img \

##############################################################################

./system/CONFIG.AT: ./bin/dy-conf$(DANCY_EXE)
	$(DY_CONF) -o$@ 1280x1024x32

./system/DB_000.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_000_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_000_FILES)

./system/DB_100.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_100_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_100_FILES)

./system/DB_200.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_200_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_200_FILES)

./system/DB_300.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_300_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_300_FILES)

./system/DB_400.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_400_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_400_FILES)

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tloader $@
