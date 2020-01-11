# Dancy Operating System

DANCY_DY=./bin/dy
DANCY_EXE=
DANCY_EXT=./external/external.sh

include ./VERSION
include ./scripts/header.mk

DANCY_MK=touch ./scripts/dancy.mk
DANCY_PATH=`./scripts/path.sh`

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=-O2 -std=c89 $(DANCY_WARNINGS)
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

DANCY_AS=PATH=$(DANCY_PATH) `./scripts/asm.sh`
DANCY_A32=$(DANCY_AS) -fwin32 -o
DANCY_A64=$(DANCY_AS) -fwin64 -o

DANCY_CC=PATH=$(DANCY_PATH) `./scripts/gcc.sh`
DANCY_O32=$(DANCY_CC) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -m32 -o
DANCY_O64=$(DANCY_CC) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -m64 -o

ACPICA_SOURCE=bash ./scripts/acpica.sh
ACPICA_O32=$(DANCY_CC) $(ACPICA_CPPFLAGS_32) $(ACPICA_CFLAGS_32) -c -m32 -o
ACPICA_O64=$(DANCY_CC) $(ACPICA_CPPFLAGS_64) $(ACPICA_CFLAGS_64) -c -m64 -o

ifdef DANCY_WITH_ACPICA
ACPICA_O32_AT=./o32/acpica.at
ACPICA_O64_AT=./o64/acpica.at
else
ACPICA_O32_AT=
ACPICA_O64_AT=
endif

all: all-release

all-release: $(DANCY_TARGET_RELEASE)

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

clean:
	@bash scripts/clean.sh

distclean: clean
	@rm -rf external

path: ./bin/dy-path$(DANCY_EXE)
	@PATH=$(DANCY_PATH) ./bin/dy-path$(DANCY_EXE)

include ./scripts/acpica.mk
include ./scripts/dirs.mk
include ./scripts/external.mk
include ./scripts/init.mk
include ./scripts/legacy.mk
include ./scripts/objects.mk
include ./scripts/release.mk
include ./scripts/system.mk
include ./scripts/tools.mk
include ./scripts/uefi.mk
