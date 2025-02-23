# Dancy Operating System

##############################################################################

ARCTIC_BIN32_FILES= \
 ./arctic/bin32/hello \
 ./arctic/bin32/cat \
 ./arctic/bin32/cp \
 ./arctic/bin32/date \
 ./arctic/bin32/dsh \
 ./arctic/bin32/env \
 ./arctic/bin32/false \
 ./arctic/bin32/hd \
 ./arctic/bin32/hexdump \
 ./arctic/bin32/init \
 ./arctic/bin32/ls \
 ./arctic/bin32/lsusb \
 ./arctic/bin32/mkdir \
 ./arctic/bin32/more \
 ./arctic/bin32/poweroff \
 ./arctic/bin32/ps \
 ./arctic/bin32/pwd \
 ./arctic/bin32/reboot \
 ./arctic/bin32/rm \
 ./arctic/bin32/rmdir \
 ./arctic/bin32/sleep \
 ./arctic/bin32/terminal \
 ./arctic/bin32/true \

##############################################################################

./arctic/bin32.img: $(ARCTIC_BIN32_FILES)
	$(DY_VBR) -t ramfs $@ 2880
	$(DY_MCOPY) -i $@ ./arctic/bin32/hello ::hello
	$(DY_MCOPY) -i $@ ./arctic/bin32/cat ::cat
	$(DY_MCOPY) -i $@ ./arctic/bin32/cp ::cp
	$(DY_MCOPY) -i $@ ./arctic/bin32/date ::date
	$(DY_MCOPY) -i $@ ./arctic/bin32/dsh ::dsh
	$(DY_MCOPY) -i $@ ./arctic/bin32/env ::env
	$(DY_MCOPY) -i $@ ./arctic/bin32/false ::false
	$(DY_MCOPY) -i $@ ./arctic/bin32/hd ::hd
	$(DY_MCOPY) -i $@ ./arctic/bin32/hexdump ::hexdump
	$(DY_MCOPY) -i $@ ./arctic/bin32/init ::init
	$(DY_MCOPY) -i $@ ./arctic/bin32/ls ::ls
	$(DY_MCOPY) -i $@ ./arctic/bin32/lsusb ::lsusb
	$(DY_MCOPY) -i $@ ./arctic/bin32/mkdir ::mkdir
	$(DY_MCOPY) -i $@ ./arctic/bin32/more ::more
	$(DY_MCOPY) -i $@ ./arctic/bin32/poweroff ::poweroff
	$(DY_MCOPY) -i $@ ./arctic/bin32/ps ::ps
	$(DY_MCOPY) -i $@ ./arctic/bin32/pwd ::pwd
	$(DY_MCOPY) -i $@ ./arctic/bin32/reboot ::reboot
	$(DY_MCOPY) -i $@ ./arctic/bin32/rm ::rm
	$(DY_MCOPY) -i $@ ./arctic/bin32/rmdir ::rmdir
	$(DY_MCOPY) -i $@ ./arctic/bin32/sleep ::sleep
	$(DY_MCOPY) -i $@ ./arctic/bin32/terminal ::terminal
	$(DY_MCOPY) -i $@ ./arctic/bin32/true ::true
