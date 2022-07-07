# Dancy Operating System

##############################################################################

ARCTIC_BIN32_FILES= \
 ./arctic/bin32/hello

##############################################################################

./arctic/bin32.img: $(ARCTIC_BIN32_FILES)
	$(DY_VBR) -t floppy $@ 720
	$(DY_MCOPY) -i $@ ./arctic/bin32/hello ::hello
