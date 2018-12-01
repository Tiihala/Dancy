# Dancy Operating System

DANCY_VERSION=-DDANCY_MAJOR=$(DANCY_MAJOR) -DDANCY_MINOR=$(DANCY_MINOR)

DANCY_CPPFLAGS=-I./include $(DANCY_VERSION)
DANCY_CFLAGS=-ffreestanding -O2
DANCY_WARNINGS=-Wall -Wextra -Wshadow -Wwrite-strings -pedantic

DANCY_CPPFLAGS_32=$(DANCY_CPPFLAGS) -DDANCY_32=1
DANCY_CPPFLAGS_64=$(DANCY_CPPFLAGS) -DDANCY_64=1
DANCY_CFLAGS_32=$(DANCY_CFLAGS) $(DANCY_WARNINGS)
DANCY_CFLAGS_64=$(DANCY_CFLAGS) $(DANCY_WARNINGS)

DANCY_TARGET_TOOLS= \
 ./bin/dy-blob$(DANCY_EXE) \
 ./bin/dy-gpt$(DANCY_EXE) \
 ./bin/dy-init$(DANCY_EXE) \
 ./bin/dy-link$(DANCY_EXE) \
 ./bin/dy-mbr$(DANCY_EXE) \
 ./bin/dy-path$(DANCY_EXE) \
 ./bin/dy-vbr$(DANCY_EXE)

DANCY_TARGET_SYSTEM= \
 ./system/IN_IA32.AT \
 ./system/IN_X64.AT \
 ./LOADER.512 \
 ./LOADER.AT

DANCY_TARGET_RELEASE=

DANCY_HEADERS= \
 ./include/dancy/limits.h \
 ./include/dancy/types.h \
 ./include/dancy.h
