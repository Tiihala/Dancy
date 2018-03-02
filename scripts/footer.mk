# Dancy Operating System

##############################################################################

DY_BLOB_OBJECTS= \
 ./tools/dy-blob/dy-blob.obj \
 ./boot/fat/ldr512.obj \
 ./loader/loader.obj

./bin/dy-blob$(DANCY_EXE): $(DY_BLOB_OBJECTS)
	$(DANCY_HOST_BINARY)$@ $(DY_BLOB_OBJECTS)

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

./tools/dy-mbr/dy-mbr.obj: ./tools/dy-mbr/dy-mbr.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-mbr/dy-mbr.c

./tools/dy-vbr/dy-vbr.obj: ./tools/dy-vbr/dy-vbr.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-vbr/dy-vbr.c
