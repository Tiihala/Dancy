# Dancy Operating System

DANCY_DY=bin\dy
DANCY_EXE=.exe
DANCY_EXT=

DANCY_AS=nasm.exe
DANCY_A32=$(DANCY_AS) -fwin32 -o
DANCY_A64=$(DANCY_AS) -fwin64 -o

DANCY_CC=clang.exe
DANCY_C32=$(DANCY_CC) -m32
DANCY_C64=$(DANCY_CC) -m64

ACPICA_SOURCE=call scripts\git\acpica.cmd
SORTIX_SOURCE=call scripts\git\sortix.cmd

DANCY_MK=copy NUL scripts\dancy.mk
DANCY_UP=call scripts\git\_up.cmd

!include .\VERSION
!include .\scripts\header.mk
!include .\scripts\deps.mk

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=/O2 /nologo
DANCY_HOST_BINARY=link.exe /NOLOGO /MANIFEST:EMBED /OUT:
DANCY_HOST_OBJECT=cl.exe /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

all: all-release

all-release: $(DANCY_TARGET_RELEASE)

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

clean:
	@cmd /C call scripts\clean.cmd

distclean: clean
	@if exist external rmdir external /S /Q

!include .\scripts\at\acpica.mk
!include .\scripts\at\acpios.mk
!include .\scripts\at\base.mk
!include .\scripts\at\debug.mk
!include .\scripts\at\lib.mk
!include .\scripts\at\misc.mk
!include .\scripts\at\network.mk
!include .\scripts\at\pci.mk
!include .\scripts\at\ps2.mk
!include .\scripts\at\run.mk
!include .\scripts\at\sched.mk
!include .\scripts\at\syscall.mk
!include .\scripts\at\usb.mk
!include .\scripts\at\vfs.mk

!include .\scripts\arctic\_bin32.mk
!include .\scripts\arctic\_bin64.mk
!include .\scripts\arctic\_root.mk
!include .\scripts\arctic\apps.mk
!include .\scripts\arctic\libc.mk
!include .\scripts\arctic\programs.mk
!include .\scripts\arctic\sortix.mk

!include .\scripts\boot\init.mk
!include .\scripts\boot\uefi.mk

!include .\scripts\dirs.mk
!include .\scripts\external.mk
!include .\scripts\legacy.mk
!include .\scripts\objects.mk
!include .\scripts\release.mk
!include .\scripts\system.mk
!include .\scripts\tools.mk
