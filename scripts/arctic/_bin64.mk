# Dancy Operating System

##############################################################################

ARCTIC_BIN64_FILES= \
 ./arctic/bin64/hello \
 ./arctic/bin64/cat \
 ./arctic/bin64/dsh \
 ./arctic/bin64/init \
 ./arctic/bin64/ls \
 ./arctic/bin64/reboot \
 ./arctic/bin64/sleep \
 ./arctic/bin64/terminal \

##############################################################################

./arctic/bin64.img: $(ARCTIC_BIN64_FILES)
	$(DY_VBR) -t ramfs $@ 720
	$(DY_MCOPY) -i $@ ./arctic/bin64/hello ::hello
	$(DY_MCOPY) -i $@ ./arctic/bin64/cat ::cat
	$(DY_MCOPY) -i $@ ./arctic/bin64/dsh ::dsh
	$(DY_MCOPY) -i $@ ./arctic/bin64/init ::init
	$(DY_MCOPY) -i $@ ./arctic/bin64/ls ::ls
	$(DY_MCOPY) -i $@ ./arctic/bin64/reboot ::reboot
	$(DY_MCOPY) -i $@ ./arctic/bin64/sleep ::sleep
	$(DY_MCOPY) -i $@ ./arctic/bin64/terminal ::terminal
