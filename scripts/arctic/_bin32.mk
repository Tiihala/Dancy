# Dancy Operating System

##############################################################################

ARCTIC_BIN32_FILES= \
 ./arctic/bin32/hello \
 ./arctic/bin32/cat \
 ./arctic/bin32/dsh \
 ./arctic/bin32/false \
 ./arctic/bin32/hexdump \
 ./arctic/bin32/init \
 ./arctic/bin32/ls \
 ./arctic/bin32/poweroff \
 ./arctic/bin32/reboot \
 ./arctic/bin32/sleep \
 ./arctic/bin32/terminal \
 ./arctic/bin32/true \

##############################################################################

./arctic/bin32.img: $(ARCTIC_BIN32_FILES)
	$(DY_VBR) -t ramfs $@ 720
	$(DY_MCOPY) -i $@ ./arctic/bin32/hello ::hello
	$(DY_MCOPY) -i $@ ./arctic/bin32/cat ::cat
	$(DY_MCOPY) -i $@ ./arctic/bin32/dsh ::dsh
	$(DY_MCOPY) -i $@ ./arctic/bin32/false ::false
	$(DY_MCOPY) -i $@ ./arctic/bin32/hexdump ::hexdump
	$(DY_MCOPY) -i $@ ./arctic/bin32/init ::init
	$(DY_MCOPY) -i $@ ./arctic/bin32/ls ::ls
	$(DY_MCOPY) -i $@ ./arctic/bin32/poweroff ::poweroff
	$(DY_MCOPY) -i $@ ./arctic/bin32/reboot ::reboot
	$(DY_MCOPY) -i $@ ./arctic/bin32/sleep ::sleep
	$(DY_MCOPY) -i $@ ./arctic/bin32/terminal ::terminal
	$(DY_MCOPY) -i $@ ./arctic/bin32/true ::true
