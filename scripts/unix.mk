# Dancy Operating System

DANCY_DY=./bin/dy
DANCY_EXE=
DANCY_EXT=./external/VERSION

DANCY_AS=bash ./scripts/asm.sh
DANCY_A32=$(DANCY_AS) -fwin32 -o
DANCY_A64=$(DANCY_AS) -fwin64 -o

DANCY_CC=bash ./scripts/cc.sh
DANCY_C32=$(DANCY_CC) -m32
DANCY_C64=$(DANCY_CC) -m64

ACPICA_SOURCE=bash ./scripts/git/acpica.sh
SORTIX_SOURCE=bash ./scripts/git/sortix.sh

DANCY_MK=touch ./scripts/dancy.mk
DANCY_UP=bash ./scripts/git/_up.sh

include ./VERSION
include ./scripts/header.mk
include ./scripts/deps.mk

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=-O2 -std=c89 $(DANCY_WARNINGS)
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

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
include ./scripts/at/network.mk
include ./scripts/at/pci.mk
include ./scripts/at/ps2.mk
include ./scripts/at/run.mk
include ./scripts/at/sched.mk
include ./scripts/at/syscall.mk
include ./scripts/at/usb.mk
include ./scripts/at/vfs.mk

include ./scripts/arctic/_bin32.mk
include ./scripts/arctic/_bin64.mk
include ./scripts/arctic/_root.mk
include ./scripts/arctic/apps.mk
include ./scripts/arctic/libc.mk
include ./scripts/arctic/programs.mk
include ./scripts/arctic/sortix.mk

include ./scripts/boot/init.mk
include ./scripts/boot/uefi.mk

include ./scripts/dirs.mk
include ./scripts/external.mk
include ./scripts/legacy.mk
include ./scripts/objects.mk
include ./scripts/release.mk
include ./scripts/system.mk
include ./scripts/tools.mk
