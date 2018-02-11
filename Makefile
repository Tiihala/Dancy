# Dancy Operating System

.PHONY: all
all: bin/dy-mbr bin/dy-vbr bin/loader.512

bin/dy-mbr: tools/dy-mbr.c boot/mbr/mbr.c include/dancy.h
	$(CC) -o bin/dy-mbr -Iinclude -std=c89 -Wall -Wextra -pedantic \
            tools/dy-mbr.c boot/mbr/mbr.c

bin/dy-vbr: tools/dy-vbr.c boot/fat/floppy.c boot/fat/ldr512.c \
            boot/fat/vbrchs.c boot/fat/vbrlba.c include/dancy.h
	$(CC) -o bin/dy-vbr -Iinclude -std=c89 -Wall -Wextra -pedantic \
            tools/dy-vbr.c boot/fat/floppy.c boot/fat/ldr512.c \
            boot/fat/vbrchs.c boot/fat/vbrlba.c

bin/loader.512: bin/dy-vbr
	bin/dy-vbr -o bin/loader.512

.PHONY: clean
clean:
	$(RM) bin/dy-*
	$(RM) bin/fdd*.img
	$(RM) bin/hdd*.img
	$(RM) bin/loader.512

.PHONY: distclean
distclean: clean
