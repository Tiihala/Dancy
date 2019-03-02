# Dancy Operating System

DANCY_VERSION=-DDANCY_MAJOR=$(DANCY_MAJOR) -DDANCY_MINOR=$(DANCY_MINOR)

DANCY_CPPFLAGS=-I./common -I./include $(DANCY_VERSION)
DANCY_CFLAGS=-ffreestanding -O2 -mno-mmx -mno-sse -mno-sse2
DANCY_WARNINGS=-Wall -Wconversion -Wextra -Wshadow -Wwrite-strings -pedantic \
 -Wpointer-arith -Wcast-align -Wstrict-prototypes

DANCY_CPPFLAGS_32=$(DANCY_CPPFLAGS) -DDANCY_32
DANCY_CPPFLAGS_64=$(DANCY_CPPFLAGS) -DDANCY_64
DANCY_CFLAGS_32=$(DANCY_CFLAGS) $(DANCY_WARNINGS)
DANCY_CFLAGS_64=$(DANCY_CFLAGS) $(DANCY_WARNINGS)

DY_BLOB=$(DANCY_DY)-blob$(DANCY_EXE)
DY_GPT=$(DANCY_DY)-gpt$(DANCY_EXE)
DY_HACK=$(DANCY_DY)-hack$(DANCY_EXE)
DY_INIT=$(DANCY_DY)-init$(DANCY_EXE)
DY_ISO=$(DANCY_DY)-iso$(DANCY_EXE)
DY_LINK=$(DANCY_DY)-link$(DANCY_EXE)
DY_MBR=$(DANCY_DY)-mbr$(DANCY_EXE)
DY_MCOPY=$(DANCY_DY)-mcopy$(DANCY_EXE)
DY_PATH=$(DANCY_DY)-path$(DANCY_EXE)
DY_VBR=$(DANCY_DY)-vbr$(DANCY_EXE)
DY_ZIP=$(DANCY_DY)-zip$(DANCY_EXE)

DANCY_TARGET_TOOLS= \
 ./bin/dy-blob$(DANCY_EXE) \
 ./bin/dy-gpt$(DANCY_EXE) \
 ./bin/dy-hack$(DANCY_EXE) \
 ./bin/dy-init$(DANCY_EXE) \
 ./bin/dy-iso$(DANCY_EXE) \
 ./bin/dy-link$(DANCY_EXE) \
 ./bin/dy-mbr$(DANCY_EXE) \
 ./bin/dy-mcopy$(DANCY_EXE) \
 ./bin/dy-path$(DANCY_EXE) \
 ./bin/dy-vbr$(DANCY_EXE) \
 ./bin/dy-zip$(DANCY_EXE)

DANCY_TARGET_SYSTEM= \
 ./system/IN_IA16.AT \
 ./system/IN_IA32.AT \
 ./system/IN_X64.AT \
 ./LOADER.512 \
 ./LOADER.AT

DANCY_TARGET_IMAGES= \
 ./release/dancy.iso \
 ./release/fdd160.img \
 ./release/fdd720.img \
 ./release/fdd1440.img \
 ./release/usbhuge.img \
 ./release/usbtiny.img

DANCY_TARGET_RELEASE= \
 ./release/dancy.zip

DANCY_HEADERS= \
 ./common/bitarray/bitarray.h \
 ./common/huffman/huffman.h \
 ./include/dancy/blob.h \
 ./include/dancy/crc.h \
 ./include/dancy/keys.h \
 ./include/dancy/limits.h \
 ./include/dancy/stdarg.h \
 ./include/dancy/stdio.h \
 ./include/dancy/stdlib.h \
 ./include/dancy/string.h \
 ./include/dancy/types.h \
 ./include/kernel/boot.h \
 ./include/kernel/init.h \
 ./include/dancy.h

DANCY_DEPS=$(DANCY_EXT) $(DANCY_HEADERS) $(DANCY_TARGET_TOOLS)
