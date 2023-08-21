# Dancy Operating System

##############################################################################

ARCTIC_BIN64_FILES= \
 ./arctic/bin64/hello \
 ./arctic/bin64/init \
 ./arctic/bin64/terminal \

##############################################################################

./arctic/bin64.img: $(ARCTIC_BIN64_FILES)
	$(DY_VBR) -t ramfs $@ 720
	$(DY_MCOPY) -i $@ ./arctic/bin64/hello ::hello
	$(DY_MCOPY) -i $@ ./arctic/bin64/init ::init
	$(DY_MCOPY) -i $@ ./arctic/bin64/terminal ::terminal
