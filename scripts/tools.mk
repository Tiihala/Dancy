# Dancy Operating System

##############################################################################

DY_BLOB_OBJECTS= \
 ./tools/dy-blob/dy-blob.obj \
 ./boot/early/eltorito.obj \
 ./boot/early/ldr512.obj \
 ./boot/loader.obj \

./bin/dy-blob$(DANCY_EXE): $(DY_BLOB_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_BLOB_OBJECTS)

##############################################################################

DY_CONF_OBJECTS= \
 ./tools/dy-conf/dy-conf.obj \
 ./common/crc32c.obj \

./bin/dy-conf$(DANCY_EXE): $(DY_CONF_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_CONF_OBJECTS)

##############################################################################

DY_GPT_OBJECTS= \
 ./tools/dy-gpt/dy-gpt.obj \
 ./boot/early/gpt.obj \
 ./common/crc32c.obj \

./bin/dy-gpt$(DANCY_EXE): $(DY_GPT_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_GPT_OBJECTS)

##############################################################################

DY_INIT_OBJECTS= \
 ./tools/dy-init/dy-init.obj \
 ./tools/dy-init/ia32.obj \
 ./tools/dy-init/x64.obj \

./bin/dy-init$(DANCY_EXE): $(DY_INIT_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_INIT_OBJECTS)

##############################################################################

DY_ISO_OBJECTS= \
 ./tools/dy-iso/dy-iso.obj \
 ./boot/early/eltorito.obj \

./bin/dy-iso$(DANCY_EXE): $(DY_ISO_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_ISO_OBJECTS)

##############################################################################

DY_LINK_OBJECTS= \
 ./tools/dy-link/main.obj \
 ./tools/dy-link/dump.obj \
 ./tools/dy-link/lib.obj \
 ./tools/dy-link/link.obj \
 ./tools/dy-link/program.obj \
 ./tools/dy-link/section.obj \
 ./tools/dy-link/symbol.obj \
 ./tools/dy-link/validate.obj \

DY_LINK_HEADERS= \
 ./tools/dy-link/program.h \

./bin/dy-link$(DANCY_EXE): $(DY_LINK_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_LINK_OBJECTS)

##############################################################################

DY_MBR_OBJECTS= \
 ./tools/dy-mbr/dy-mbr.obj \
 ./boot/early/mbr.obj \
 ./common/crc32c.obj \

./bin/dy-mbr$(DANCY_EXE): $(DY_MBR_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_MBR_OBJECTS)

##############################################################################

DY_MCOPY_OBJECTS= \
 ./tools/dy-mcopy/main.obj \
 ./tools/dy-mcopy/mcopy.obj \
 ./tools/dy-mcopy/program.obj \
 ./common/fat.obj \

DY_MCOPY_HEADERS= \
 ./tools/dy-mcopy/program.h \

./bin/dy-mcopy$(DANCY_EXE): $(DY_MCOPY_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_MCOPY_OBJECTS)

##############################################################################

DY_PATCH_OBJECTS= \
 ./tools/dy-patch/main.obj \
 ./tools/dy-patch/patch.obj \
 ./tools/dy-patch/program.obj \

DY_PATCH_HEADERS= \
 ./tools/dy-patch/program.h \

./bin/dy-patch$(DANCY_EXE): $(DY_PATCH_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_PATCH_OBJECTS)

##############################################################################

DY_PATH_OBJECTS= \
 ./tools/dy-path/dy-path.obj \

./bin/dy-path$(DANCY_EXE): $(DY_PATH_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_PATH_OBJECTS)

##############################################################################

DY_PNG_OBJECTS= \
 ./tools/dy-png/main.obj \
 ./tools/dy-png/bitarray.obj \
 ./tools/dy-png/convert.obj \
 ./tools/dy-png/deflate.obj \
 ./tools/dy-png/huffman.obj \
 ./tools/dy-png/palette.obj \
 ./tools/dy-png/program.obj \
 ./common/crc32.obj \

DY_PNG_HEADERS= \
 ./tools/dy-png/program.h \

./bin/dy-png$(DANCY_EXE): $(DY_PNG_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_PNG_OBJECTS)

##############################################################################

DY_TTF_OBJECTS= \
 ./tools/dy-ttf/main.obj \
 ./tools/dy-ttf/program.obj \
 ./tools/dy-ttf/render.obj \
 ./tools/dy-ttf/table.obj \
 ./tools/dy-ttf/ttf.obj \

DY_TTF_HEADERS= \
 ./tools/dy-ttf/program.h \

./bin/dy-ttf$(DANCY_EXE): $(DY_TTF_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_TTF_OBJECTS)

##############################################################################

DY_UEFI_OBJECTS= \
 ./tools/dy-uefi/dy-uefi.obj \
 ./tools/dy-uefi/x64.obj \

./bin/dy-uefi$(DANCY_EXE): $(DY_UEFI_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_UEFI_OBJECTS)

##############################################################################

DY_VBR_OBJECTS= \
 ./tools/dy-vbr/dy-vbr.obj \
 ./boot/early/floppy.obj \
 ./boot/early/ldr512.obj \
 ./boot/early/vbrchs.obj \
 ./boot/early/vbrlba.obj \
 ./common/crc32c.obj \

./bin/dy-vbr$(DANCY_EXE): $(DY_VBR_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_VBR_OBJECTS)

##############################################################################

DY_ZIP_OBJECTS= \
 ./tools/dy-zip/main.obj \
 ./tools/dy-zip/bitarray.obj \
 ./tools/dy-zip/deflate.obj \
 ./tools/dy-zip/huffman.obj \
 ./tools/dy-zip/program.obj \
 ./common/crc32.obj \

./bin/dy-zip$(DANCY_EXE): $(DY_ZIP_OBJECTS) ./scripts/dancy.mk
	$(DANCY_HOST_BINARY)$@ $(DY_ZIP_OBJECTS)

##############################################################################

./boot/early/eltorito.obj: ./boot/early/eltorito.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/eltorito.c

./boot/early/floppy.obj: ./boot/early/floppy.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/floppy.c

./boot/early/gpt.obj: ./boot/early/gpt.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/gpt.c

./boot/early/ldr512.obj: ./boot/early/ldr512.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/ldr512.c

./boot/early/mbr.obj: ./boot/early/mbr.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/mbr.c

./boot/early/vbrchs.obj: ./boot/early/vbrchs.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/vbrchs.c

./boot/early/vbrlba.obj: ./boot/early/vbrlba.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/early/vbrlba.c

./boot/loader.obj: ./boot/loader.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./boot/loader.c

./common/crc32.obj: ./common/crc32.c
	$(DANCY_HOST_OBJECT)$@ ./common/crc32.c

./common/crc32c.obj: ./common/crc32c.c
	$(DANCY_HOST_OBJECT)$@ ./common/crc32c.c

./common/fat.obj: ./common/fat.c
	$(DANCY_HOST_OBJECT)$@ ./common/fat.c

./tools/dy-blob/dy-blob.obj: ./tools/dy-blob/dy-blob.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-blob/dy-blob.c

./tools/dy-conf/dy-conf.obj: ./tools/dy-conf/dy-conf.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-conf/dy-conf.c

./tools/dy-gpt/dy-gpt.obj: ./tools/dy-gpt/dy-gpt.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-gpt/dy-gpt.c

./tools/dy-init/dy-init.obj: ./tools/dy-init/dy-init.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-init/dy-init.c

./tools/dy-init/ia32.obj: ./tools/dy-init/ia32.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-init/ia32.c

./tools/dy-init/x64.obj: ./tools/dy-init/x64.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-init/x64.c

./tools/dy-iso/dy-iso.obj: ./tools/dy-iso/dy-iso.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-iso/dy-iso.c

./tools/dy-link/dump.obj: ./tools/dy-link/dump.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/dump.c

./tools/dy-link/lib.obj: ./tools/dy-link/lib.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/lib.c

./tools/dy-link/link.obj: ./tools/dy-link/link.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/link.c

./tools/dy-link/main.obj: ./tools/dy-link/main.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/main.c

./tools/dy-link/program.obj: ./tools/dy-link/program.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/program.c

./tools/dy-link/section.obj: ./tools/dy-link/section.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/section.c

./tools/dy-link/symbol.obj: ./tools/dy-link/symbol.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/symbol.c

./tools/dy-link/validate.obj: ./tools/dy-link/validate.c $(DY_LINK_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-link/validate.c

./tools/dy-mbr/dy-mbr.obj: ./tools/dy-mbr/dy-mbr.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-mbr/dy-mbr.c

./tools/dy-mcopy/main.obj: ./tools/dy-mcopy/main.c $(DY_MCOPY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-mcopy/main.c

./tools/dy-mcopy/mcopy.obj: ./tools/dy-mcopy/mcopy.c $(DY_MCOPY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-mcopy/mcopy.c

./tools/dy-mcopy/program.obj: ./tools/dy-mcopy/program.c $(DY_MCOPY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-mcopy/program.c

./tools/dy-patch/main.obj: ./tools/dy-patch/main.c $(DY_PATCH_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-patch/main.c

./tools/dy-patch/patch.obj: ./tools/dy-patch/patch.c $(DY_PATCH_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-patch/patch.c

./tools/dy-patch/program.obj: ./tools/dy-patch/program.c $(DY_PATCH_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-patch/program.c

./tools/dy-path/dy-path.obj: ./tools/dy-path/dy-path.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-path/dy-path.c

./tools/dy-png/bitarray.obj: ./tools/dy-png/bitarray.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/bitarray.c

./tools/dy-png/convert.obj: ./tools/dy-png/convert.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/convert.c

./tools/dy-png/deflate.obj: ./tools/dy-png/deflate.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/deflate.c

./tools/dy-png/huffman.obj: ./tools/dy-png/huffman.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/huffman.c

./tools/dy-png/main.obj: ./tools/dy-png/main.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/main.c

./tools/dy-png/palette.obj: ./tools/dy-png/palette.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/palette.c

./tools/dy-png/program.obj: ./tools/dy-png/program.c $(DY_PNG_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-png/program.c

./tools/dy-ttf/main.obj: ./tools/dy-ttf/main.c $(DY_TTF_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-ttf/main.c

./tools/dy-ttf/program.obj: ./tools/dy-ttf/program.c $(DY_TTF_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-ttf/program.c

./tools/dy-ttf/render.obj: ./tools/dy-ttf/render.c $(DY_TTF_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-ttf/render.c

./tools/dy-ttf/table.obj: ./tools/dy-ttf/table.c $(DY_TTF_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-ttf/table.c

./tools/dy-ttf/ttf.obj: ./tools/dy-ttf/ttf.c $(DY_TTF_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-ttf/ttf.c

./tools/dy-uefi/dy-uefi.obj: ./tools/dy-uefi/dy-uefi.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-uefi/dy-uefi.c

./tools/dy-uefi/x64.obj: ./tools/dy-uefi/x64.c $(DANCY_HEADERS)
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-uefi/x64.c

./tools/dy-vbr/dy-vbr.obj: ./tools/dy-vbr/dy-vbr.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-vbr/dy-vbr.c

./tools/dy-zip/bitarray.obj: ./tools/dy-zip/bitarray.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-zip/bitarray.c

./tools/dy-zip/deflate.obj: ./tools/dy-zip/deflate.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-zip/deflate.c

./tools/dy-zip/huffman.obj: ./tools/dy-zip/huffman.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-zip/huffman.c

./tools/dy-zip/main.obj: ./tools/dy-zip/main.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-zip/main.c

./tools/dy-zip/program.obj: ./tools/dy-zip/program.c
	$(DANCY_HOST_OBJECT)$@ ./tools/dy-zip/program.c
