# Dancy Operating System

##############################################################################

ARCTIC_ROOT_FILES= \
 ./arctic/include/stdio.h \
 ./share/fonts/dcysan.ttf \
 ./share/fonts/dcysanb.ttf \
 ./share/fonts/dcysanm.ttf \

##############################################################################

./arctic/root.img: $(ARCTIC_ROOT_FILES) $(DANCY_TARGET_TOOLS)
	$(DY_VBR) -t ramfs $@ 720
	$(DY_MCOPY) -i $@ ./arctic/include/stdio.h ::include/stdio.h
	$(DY_MCOPY) -i $@ ./share/fonts/dcysan.ttf ::share/fonts/dcysan.ttf
	$(DY_MCOPY) -i $@ ./share/fonts/dcysanb.ttf ::share/fonts/dcysanb.ttf
	$(DY_MCOPY) -i $@ ./share/fonts/dcysanm.ttf ::share/fonts/dcysanm.ttf
