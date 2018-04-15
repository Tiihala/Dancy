# Dancy Operating System

##############################################################################

DY_BLOB_OBJECTS= \
 ./tools/dy-blob/dy-blob.obj \
 ./boot/cd/eltorito.obj \
 ./boot/fat/ldr512.obj \
 ./loader/loader.obj

./bin/dy-blob$(DANCY_EXE): $(DY_BLOB_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_BLOB_OBJECTS)

##############################################################################

DY_GPT_OBJECTS= \
 ./tools/dy-gpt/dy-gpt.obj \
 ./boot/gpt/gpt.obj \
 ./common/crc32/crc32c.obj

./bin/dy-gpt$(DANCY_EXE): $(DY_GPT_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_GPT_OBJECTS)

##############################################################################

DY_INIT_OBJECTS= \
 ./tools/dy-init/dy-init.obj

./bin/dy-init$(DANCY_EXE): $(DY_INIT_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_INIT_OBJECTS)

##############################################################################

DY_LINK_OBJECTS= \
 ./tools/dy-link/main.obj \
 ./tools/dy-link/dump.obj \
 ./tools/dy-link/program.obj \
 ./tools/dy-link/validate.obj
DY_LINK_HEADERS= \
 ./tools/dy-link/program.h

./bin/dy-link$(DANCY_EXE): $(DY_LINK_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_LINK_OBJECTS)

##############################################################################

DY_MBR_OBJECTS= \
 ./tools/dy-mbr/dy-mbr.obj \
 ./boot/mbr/mbr.obj \
 ./common/crc32/crc32c.obj

./bin/dy-mbr$(DANCY_EXE): $(DY_MBR_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_MBR_OBJECTS)

##############################################################################

DY_VBR_OBJECTS= \
 ./tools/dy-vbr/dy-vbr.obj \
 ./boot/fat/floppy.obj \
 ./boot/fat/ldr512.obj \
 ./boot/fat/vbrchs.obj \
 ./boot/fat/vbrlba.obj \
 ./common/crc32/crc32c.obj

./bin/dy-vbr$(DANCY_EXE): $(DY_VBR_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_VBR_OBJECTS)

##############################################################################

./boot/cd/eltorito.obj: ./boot/cd/eltorito.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/cd/eltorito.c

./boot/fat/floppy.obj: ./boot/fat/floppy.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/fat/floppy.c

./boot/fat/ldr512.obj: ./boot/fat/ldr512.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/fat/ldr512.c

./boot/fat/vbrchs.obj: ./boot/fat/vbrchs.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/fat/vbrchs.c

./boot/fat/vbrlba.obj: ./boot/fat/vbrlba.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/fat/vbrlba.c

./boot/gpt/gpt.obj: ./boot/gpt/gpt.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/gpt/gpt.c

./boot/mbr/mbr.obj: ./boot/mbr/mbr.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./boot/mbr/mbr.c

./common/crc32/crc32.obj: ./common/crc32/crc32.c
	$(DANCY_HOST_OBJECT)$@ ./common/crc32/crc32.c

./common/crc32/crc32c.obj: ./common/crc32/crc32c.c
	$(DANCY_HOST_OBJECT)$@ ./common/crc32/crc32c.c

./loader/loader.obj: ./loader/loader.c ./include/dancy.h
	$(DANCY_HOST_OBJECT)$@ ./loader/loader.c

./tools/dy-blob/dy-blob.obj: ./tools/dy-blob/dy-blob.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-blob/dy-blob.c

./tools/dy-gpt/dy-gpt.obj: ./tools/dy-gpt/dy-gpt.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-gpt/dy-gpt.c

./tools/dy-init/dy-init.obj: ./tools/dy-init/dy-init.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-init/dy-init.c

./tools/dy-link/dump.obj: ./tools/dy-link/dump.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/dump.c

./tools/dy-link/main.obj: ./tools/dy-link/main.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/main.c

./tools/dy-link/program.obj: ./tools/dy-link/program.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/program.c

./tools/dy-link/validate.obj: ./tools/dy-link/validate.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/validate.c

./tools/dy-mbr/dy-mbr.obj: ./tools/dy-mbr/dy-mbr.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-mbr/dy-mbr.c

./tools/dy-vbr/dy-vbr.obj: ./tools/dy-vbr/dy-vbr.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-vbr/dy-vbr.c
