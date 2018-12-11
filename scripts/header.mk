# Dancy Operating System

DANCY_VERSION=-DDANCY_MAJOR=$(DANCY_MAJOR) -DDANCY_MINOR=$(DANCY_MINOR)

DANCY_CPPFLAGS=-I./include $(DANCY_VERSION)
DANCY_CFLAGS=-ffreestanding -O2
DANCY_WARNINGS=-Wall -Wextra -Wshadow -Wwrite-strings -pedantic

DANCY_CPPFLAGS_32=$(DANCY_CPPFLAGS) -DDANCY_32
DANCY_CPPFLAGS_64=$(DANCY_CPPFLAGS) -DDANCY_64
DANCY_CFLAGS_32=$(DANCY_CFLAGS) $(DANCY_WARNINGS)
DANCY_CFLAGS_64=$(DANCY_CFLAGS) $(DANCY_WARNINGS)

DY_BLOB=$(DANCY_DY)-blob$(DANCY_EXE)
DY_GPT=$(DANCY_DY)-gpt$(DANCY_EXE)
DY_INIT=$(DANCY_DY)-init$(DANCY_EXE)
DY_LINK=$(DANCY_DY)-link$(DANCY_EXE)
DY_MBR=$(DANCY_DY)-mbr$(DANCY_EXE)
DY_MCOPY=$(DANCY_DY)-mcopy$(DANCY_EXE)
DY_PATH=$(DANCY_DY)-path$(DANCY_EXE)
DY_VBR=$(DANCY_DY)-vbr$(DANCY_EXE)

DANCY_TARGET_TOOLS= \
 $(DY_BLOB) \
 $(DY_GPT) \
 $(DY_INIT) \
 $(DY_LINK) \
 $(DY_MBR) \
 $(DY_MCOPY) \
 $(DY_PATH) \
 $(DY_VBR)

DANCY_TARGET_SYSTEM= \
 ./system/IN_IA32.AT \
 ./system/IN_X64.AT \
 ./LOADER.512 \
 ./LOADER.AT

DANCY_TARGET_RELEASE= \
 ./release/fdd720.img \
 ./release/fdd1440.img

DANCY_HEADERS= \
 ./include/dancy/limits.h \
 ./include/dancy/stdarg.h \
 ./include/dancy/types.h \
 ./include/dancy.h
