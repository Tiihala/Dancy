# Dancy Operating System

DANCY_DY=bin\dy
DANCY_EXE=.exe
DANCY_EXT=

!include .\VERSION
!include .\scripts\header.mk

DANCY_MK=copy NUL scripts\dancy.mk

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=/O2 /nologo
DANCY_HOST_BINARY=link.exe /NOLOGO /MANIFEST:EMBED /OUT:
DANCY_HOST_OBJECT=cl.exe /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

DANCY_AS=nasm.exe
DANCY_A32=nasm.exe -fwin32 -o
DANCY_A64=nasm.exe -fwin64 -o

DANCY_C32=clang.exe -m32
DANCY_C64=clang.exe -m64
DANCY_O32=$(DANCY_C32) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -o
DANCY_O64=$(DANCY_C64) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -o

ACPICA_SOURCE=call scripts\acpica.cmd
ACPICA_O32=$(DANCY_C32) $(ACPICA_CPPFLAGS_32) $(ACPICA_CFLAGS_32) -c -o
ACPICA_O64=$(DANCY_C64) $(ACPICA_CPPFLAGS_64) $(ACPICA_CFLAGS_64) -c -o

ARCTIC_O32=$(DANCY_C32) $(ARCTIC_CPPFLAGS_32) $(ARCTIC_CFLAGS_32) -c -o
ARCTIC_O64=$(DANCY_C64) $(ARCTIC_CPPFLAGS_64) $(ARCTIC_CFLAGS_64) -c -o

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
!include .\scripts\at\pci.mk
!include .\scripts\at\ps2.mk
!include .\scripts\at\sched.mk
!include .\scripts\at\syscall.mk
!include .\scripts\at\vfs.mk

!include .\scripts\arctic\_bin32.mk
!include .\scripts\arctic\_bin64.mk
!include .\scripts\arctic\_root.mk
!include .\scripts\arctic\hello.mk
!include .\scripts\arctic\libc.mk

!include .\scripts\boot\init.mk
!include .\scripts\boot\uefi.mk

!include .\scripts\objects\_arctic.mk
!include .\scripts\objects\_boot.mk
!include .\scripts\objects\_common.mk
!include .\scripts\objects\kernel.mk

!include .\scripts\dirs.mk
!include .\scripts\external.mk
!include .\scripts\legacy.mk
!include .\scripts\release.mk
!include .\scripts\system.mk
!include .\scripts\tools.mk
