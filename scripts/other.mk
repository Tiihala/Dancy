# Dancy Operating System

DANCY_DY=bin\dy
DANCY_EXE=.exe
DANCY_PATH=$(MAKEDIR)\bin;$(PATH)

!include .\VERSION
!include .\scripts\header.mk

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=/O2 /Wall /wd4711 /wd4996 /nologo
DANCY_HOST_BINARY=cl.exe /Fe
DANCY_HOST_OBJECT=cl.exe /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

DANCY_AS=nasm.exe
DANCY_CC=clang.exe
DANCY_OBJECT_32=$(DANCY_CC) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -m32 -o
DANCY_OBJECT_64=$(DANCY_CC) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -m64 -o

all: all-release

all-release: $(DANCY_TARGET_RELEASE)

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

bin:
	@mkdir "bin"

clean:
	@if exist bin rmdir bin /S /Q
	@if exist o32 rmdir o32 /S /Q
	@if exist o64 rmdir o64 /S /Q
	@if exist release rmdir release /S /Q
	@if exist system rmdir system /S /Q
	@for %i in (LOADER.*) do del "%i"
	@for /R %i in (*.obj) do del "%i"

distclean: clean
	@if exist external rmdir external /S /Q

external:
	@mkdir "external"

path: ./bin/dy-path$(DANCY_EXE)
	@cmd /C "set PATH=$(DANCY_PATH) && bin\dy-path$(DANCY_EXE)"

!include .\scripts\release.mk
!include .\scripts\system.mk
!include .\scripts\tools.mk
