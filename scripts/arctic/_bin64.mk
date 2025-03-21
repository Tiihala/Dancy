# Dancy Operating System

##############################################################################

ARCTIC_BIN64_FILES= \
 ./arctic/bin64/hello \
 ./arctic/bin64/cat \
 ./arctic/bin64/cp \
 ./arctic/bin64/date \
 ./arctic/bin64/dsh \
 ./arctic/bin64/env \
 ./arctic/bin64/false \
 ./arctic/bin64/hd \
 ./arctic/bin64/hexdump \
 ./arctic/bin64/init \
 ./arctic/bin64/ls \
 ./arctic/bin64/lsusb \
 ./arctic/bin64/mkdir \
 ./arctic/bin64/more \
 ./arctic/bin64/poweroff \
 ./arctic/bin64/ps \
 ./arctic/bin64/pwd \
 ./arctic/bin64/reboot \
 ./arctic/bin64/rm \
 ./arctic/bin64/rmdir \
 ./arctic/bin64/sleep \
 ./arctic/bin64/terminal \
 ./arctic/bin64/true \

##############################################################################

./arctic/bin64.img: $(ARCTIC_BIN64_FILES)
	$(DY_VBR) -t ramfs $@ 2880
	$(DY_MCOPY) -i $@ ./arctic/bin64/hello ::hello
	$(DY_MCOPY) -i $@ ./arctic/bin64/cat ::cat
	$(DY_MCOPY) -i $@ ./arctic/bin64/cp ::cp
	$(DY_MCOPY) -i $@ ./arctic/bin64/date ::date
	$(DY_MCOPY) -i $@ ./arctic/bin64/dsh ::dsh
	$(DY_MCOPY) -i $@ ./arctic/bin64/env ::env
	$(DY_MCOPY) -i $@ ./arctic/bin64/false ::false
	$(DY_MCOPY) -i $@ ./arctic/bin64/hd ::hd
	$(DY_MCOPY) -i $@ ./arctic/bin64/hexdump ::hexdump
	$(DY_MCOPY) -i $@ ./arctic/bin64/init ::init
	$(DY_MCOPY) -i $@ ./arctic/bin64/ls ::ls
	$(DY_MCOPY) -i $@ ./arctic/bin64/lsusb ::lsusb
	$(DY_MCOPY) -i $@ ./arctic/bin64/mkdir ::mkdir
	$(DY_MCOPY) -i $@ ./arctic/bin64/more ::more
	$(DY_MCOPY) -i $@ ./arctic/bin64/poweroff ::poweroff
	$(DY_MCOPY) -i $@ ./arctic/bin64/ps ::ps
	$(DY_MCOPY) -i $@ ./arctic/bin64/pwd ::pwd
	$(DY_MCOPY) -i $@ ./arctic/bin64/reboot ::reboot
	$(DY_MCOPY) -i $@ ./arctic/bin64/rm ::rm
	$(DY_MCOPY) -i $@ ./arctic/bin64/rmdir ::rmdir
	$(DY_MCOPY) -i $@ ./arctic/bin64/sleep ::sleep
	$(DY_MCOPY) -i $@ ./arctic/bin64/terminal ::terminal
	$(DY_MCOPY) -i $@ ./arctic/bin64/true ::true
