# Dancy Operating System

##############################################################################

ARCTIC_BIN32_FILES= \
 ./arctic/bin32/hello \
 ./arctic/bin32/init \

##############################################################################

./arctic/bin32.img: $(ARCTIC_BIN32_FILES)
	$(DY_VBR) -t ramfs $@ 720
	$(DY_MCOPY) -i $@ ./arctic/bin32/hello ::hello
	$(DY_MCOPY) -i $@ ./arctic/bin32/init ::init
