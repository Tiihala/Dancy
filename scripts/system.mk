# Dancy Operating System

##############################################################################

DANCY_DB_FILES= \
 ./share/fonts/dancy.ttf \
 $(ACPICA_O32_AT) \
 ./o32/init.at \
 $(ACPICA_O64_AT) \
 ./o64/init.at

##############################################################################

./system/CONFIG.AT: ./bin/dy-conf$(DANCY_EXE)
	$(DY_CONF) -o$@

./system/DB_000.AT: ./bin/dy-zip$(DANCY_EXE) $(DANCY_DB_FILES)
	$(DY_ZIP) -o$@ $(DANCY_DB_FILES)

./system/IN_IA32.AT: $(DANCY_START_OBJECTS_32)
	$(DY_LINK) -o$@ -finit --export-all $(DANCY_START_OBJECTS_32)
	$(DY_INIT) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_START_OBJECTS_64)
	$(DY_LINK) -o$@ -finit --export-all $(DANCY_START_OBJECTS_64)
	$(DY_INIT) -tx64 --set-header $@

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	$(DY_BLOB) -tloader $@
