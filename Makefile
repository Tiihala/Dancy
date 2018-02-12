# Dancy Operating System

DANCY_MAJOR=0
DANCY_MINOR=0
DANCY_PATCH=0
DANCY_STATE=preliminary

DANCY_ALL_TARGET=LOADER.512 \
 bin/dy-mbr \
 bin/dy-vbr
DANCY_BIN_CFLAGS=-O2 -std=c89 -Wall -Wextra -pedantic

.PHONY: all
all: $(DANCY_ALL_TARGET)

SRC_DY_MBR=tools/dy-mbr.c \
 boot/mbr/mbr.c
bin/dy-mbr: $(SRC_DY_MBR) include/dancy.h
	$(CC) -o bin/dy-mbr $(DANCY_BIN_CFLAGS) -Iinclude $(SRC_DY_MBR)

SRC_DY_VBR=tools/dy-vbr.c \
 boot/fat/floppy.c \
 boot/fat/ldr512.c \
 boot/fat/vbrchs.c \
 boot/fat/vbrlba.c
bin/dy-vbr: $(SRC_DY_VBR) include/dancy.h
	$(CC) -o bin/dy-vbr $(DANCY_BIN_CFLAGS) -Iinclude $(SRC_DY_VBR)

LOADER.512: bin/dy-vbr
	bin/dy-vbr -o LOADER.512

.PHONY: clean
clean:
	$(RM) $(DANCY_ALL_TARGET)

.PHONY: distclean
distclean: clean
