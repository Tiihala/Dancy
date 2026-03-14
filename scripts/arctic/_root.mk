# Dancy Operating System

##############################################################################

./arctic/root.img: $(DANCY_TARGET_TOOLS)
	$(DY_VBR) -t ramfs $@ 720
