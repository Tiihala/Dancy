# Dancy Operating System

include ./VERSION
include ./scripts/header.mk
PATH=$(shell ./scripts/path.sh)

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=-O2 -std=c89 -Wall -Wextra -Wshadow -Wwrite-strings -pedantic

DANCY_EXE=
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

DANCY_AS=nasm
DANCY_CC=x86_64-w64-mingw32-gcc
DANCY_OBJECT_32=$(DANCY_CC) $(DANCY_CPPFLAGS) $(DANCY_CFLAGS) -c -m32 -o
DANCY_OBJECT_64=$(DANCY_CC) $(DANCY_CPPFLAGS) $(DANCY_CFLAGS) -c -m64 -o

all: all-system all-tools

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

bin:
	@mkdir bin

clean:
	@rm -rf bin
	@rm -rf o32
	@rm -rf o64
	@rm -rf system
	@rm -fv LOADER.*
	@rm -fv `find -name "*.obj"`

distclean: clean
	@rm -rf external

external:
	@bash ./scripts/external.sh

system:
	@mkdir system

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	dy-blob$(DANCY_EXE) -t loader $@

include ./scripts/system.mk
include ./scripts/tools.mk
