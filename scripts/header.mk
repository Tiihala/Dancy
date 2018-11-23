# Dancy Operating System

DANCY_VERSION=-DDANCY_MAJOR=$(DANCY_MAJOR) -DDANCY_MINOR=$(DANCY_MINOR)

DANCY_CPPFLAGS=-I./include $(DANCY_VERSION)
DANCY_CFLAGS=-ffreestanding

DANCY_TARGET_TOOLS= \
 ./bin/dy-blob$(DANCY_EXE) \
 ./bin/dy-gpt$(DANCY_EXE) \
 ./bin/dy-init$(DANCY_EXE) \
 ./bin/dy-link$(DANCY_EXE) \
 ./bin/dy-mbr$(DANCY_EXE) \
 ./bin/dy-path$(DANCY_EXE) \
 ./bin/dy-vbr$(DANCY_EXE)

DANCY_TARGET_SYSTEM= \
 ./LOADER.512 \
 ./LOADER.AT

DANCY_HEADERS= \
 ./include/dancy/limits.h \
 ./include/dancy/types.h \
 ./include/dancy.h
