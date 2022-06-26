# Dancy Operating System

DANCY_DY=./bin/dy
DANCY_EXE=
DANCY_EXT=./external/VERSION

include ./VERSION
include ./scripts/header.mk

DANCY_MK=touch ./scripts/dancy.mk

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=-O2 -std=c89 $(DANCY_WARNINGS)
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

DANCY_AS=bash ./scripts/asm.sh
DANCY_A32=$(DANCY_AS) -fwin32 -o
DANCY_A64=$(DANCY_AS) -fwin64 -o

DANCY_CC=bash ./scripts/cc.sh
DANCY_O32=$(DANCY_CC) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -o
DANCY_O64=$(DANCY_CC) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -o

ACPICA_SOURCE=bash ./scripts/acpica.sh
ACPICA_O32=$(DANCY_CC) $(ACPICA_CPPFLAGS_32) $(ACPICA_CFLAGS_32) -c -o
ACPICA_O64=$(DANCY_CC) $(ACPICA_CPPFLAGS_64) $(ACPICA_CFLAGS_64) -c -o

all: all-release

all-release: $(DANCY_TARGET_RELEASE)

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

clean:
	@bash scripts/clean.sh

distclean: clean
	@rm -rf external

include ./scripts/at/acpica.mk
include ./scripts/at/acpios.mk
include ./scripts/at/base.mk
include ./scripts/at/debug.mk
include ./scripts/at/lib.mk
include ./scripts/at/misc.mk
include ./scripts/at/pci.mk
include ./scripts/at/ps2.mk
include ./scripts/at/sched.mk
include ./scripts/at/syscall.mk
include ./scripts/at/vfs.mk

include ./scripts/boot/init.mk
include ./scripts/boot/uefi.mk

include ./scripts/objects/_boot.mk
include ./scripts/objects/_common.mk
include ./scripts/objects/objects.mk

include ./scripts/dirs.mk
include ./scripts/external.mk
include ./scripts/legacy.mk
include ./scripts/release.mk
include ./scripts/system.mk
include ./scripts/tools.mk
