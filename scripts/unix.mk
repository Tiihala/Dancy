# Dancy Operating System

include ./VERSION
include ./scripts/header.mk
DANCY_PATH=`./scripts/path.sh`

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=-O2 -std=c89 -Wall -Wextra -Wshadow -Wwrite-strings -pedantic

DANCY_EXE=
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

DANCY_AS=nasm
DANCY_CC=PATH=$(DANCY_PATH) ./external/bin/x86_64-w64-mingw32-gcc
DANCY_OBJECT_32=$(DANCY_CC) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -m32 -o
DANCY_OBJECT_64=$(DANCY_CC) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -m64 -o

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

path: ./bin/dy-path$(DANCY_EXE)
	@PATH=$(DANCY_PATH) ./bin/dy-path$(DANCY_EXE)

include ./scripts/system.mk

./system/IN_IA32.AT: $(DANCY_INIT_OBJECTS_32)
	./bin/dy-link$(DANCY_EXE) -o$@ -finit $(DANCY_INIT_OBJECTS_32)
	./bin/dy-init$(DANCY_EXE) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_INIT_OBJECTS_64)
	./bin/dy-link$(DANCY_EXE) -o$@ -finit $(DANCY_INIT_OBJECTS_64)
	./bin/dy-init$(DANCY_EXE) -tx64 --set-header $@

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t loader $@

include ./scripts/tools.mk
