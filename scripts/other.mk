# Dancy Operating System

DANCY_DY=bin\dy
DANCY_EXE=.exe
DANCY_EXT=

!include .\VERSION
!include .\scripts\header.mk

DANCY_MK=copy NUL scripts\dancy.mk
DANCY_PATH=$(MAKEDIR)\bin;$(PATH)

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=/O2 /nologo
DANCY_HOST_BINARY=link.exe /NOLOGO /MANIFEST:EMBED /OUT:
DANCY_HOST_OBJECT=cl.exe /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

DANCY_AS=nasm.exe
DANCY_A32=nasm.exe -fwin32 -o
DANCY_A64=nasm.exe -fwin64 -o

DANCY_CC=clang.exe
DANCY_O32=$(DANCY_CC) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -m32 -o
DANCY_O64=$(DANCY_CC) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -m64 -o

ACPICA_SOURCE=call scripts\acpica.cmd
ACPICA_O32=$(DANCY_CC) $(ACPICA_CPPFLAGS_32) $(ACPICA_CFLAGS_32) -c -m32 -o
ACPICA_O64=$(DANCY_CC) $(ACPICA_CPPFLAGS_64) $(ACPICA_CFLAGS_64) -c -m64 -o

all: all-release

all-release: $(DANCY_TARGET_RELEASE)

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

clean:
	@cmd /C call scripts\clean.cmd

distclean: clean
	@if exist external rmdir external /S /Q

path: ./bin/dy-path$(DANCY_EXE)
	@cmd /C "set PATH=$(DANCY_PATH) && bin\dy-path$(DANCY_EXE)"

!include .\scripts\at\acpica.mk
!include .\scripts\at\acpios.mk
!include .\scripts\at\base.mk
!include .\scripts\at\init.mk
!include .\scripts\at\lib.mk
!include .\scripts\at\uefi.mk

!include .\scripts\dirs.mk
!include .\scripts\external.mk
!include .\scripts\legacy.mk
!include .\scripts\objects.mk
!include .\scripts\release.mk
!include .\scripts\system.mk
!include .\scripts\tools.mk
