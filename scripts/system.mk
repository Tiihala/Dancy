# Dancy Operating System

##############################################################################

DANCY_DB_FILES= \
 ./o32/_boot/init.at \
 ./o32/lib.at \
 ./o32/base.at \
 ./o32/debug.at \
 ./o32/misc.at \
 ./o32/pci.at \
 ./o32/ps2.at \
 ./o32/sched.at \
 ./o32/vfs.at \
 $(ACPICA_O32_AT) \
 ./o64/_boot/init.at \
 ./o64/lib.at \
 ./o64/base.at \
 ./o64/debug.at \
 ./o64/misc.at \
 ./o64/pci.at \
 ./o64/ps2.at \
 ./o64/sched.at \
 ./o64/vfs.at \
 $(ACPICA_O64_AT) \
 ./share/fonts/dcysan.ttf \
 ./share/fonts/dcysanb.ttf \
 ./share/fonts/dcysanm.ttf \
 ./share/pictures/init.png

##############################################################################

./system/CONFIG.AT: ./bin/dy-conf$(DANCY_EXE)
	$(DY_CONF) -o$@

./system/DB_000.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_FILES)

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tloader $@
