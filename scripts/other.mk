# Dancy Operating System

!include .\VERSION
!include .\scripts\header.mk
PATH=$(MAKEDIR)\bin;$(PATH)

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=/O2 /Wall /wd4711 /wd4996 /nologo

DANCY_EXE=.exe
DANCY_HOST_BINARY=CL /Fe
DANCY_HOST_OBJECT=CL /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

DANCY_AS=NASM.EXE
DANCY_CC=CLANG.EXE
DANCY_OBJECT_32=$(DANCY_CC) $(DANCY_CPPFLAGS) $(DANCY_CFLAGS) -c -m32 -o
DANCY_OBJECT_64=$(DANCY_CC) $(DANCY_CPPFLAGS) $(DANCY_CFLAGS) -c -m64 -o

all: all-system all-tools

all-system: $(DANCY_TARGET_SYSTEM)

all-tools: $(DANCY_TARGET_TOOLS)

bin:
	@mkdir "bin"

clean:
	@if exist bin rmdir bin /S /Q
	@if exist o32 rmdir o32 /S /Q
	@if exist o64 rmdir o64 /S /Q
	@if exist system rmdir system /S /Q
	@for %i in (LOADER.*) do del "%i"
	@for /R %i in (*.obj) do del "%i"

distclean: clean
	@if exist external rmdir external /S /Q

external:
	@mkdir "external"

path: ./bin/dy-path$(DANCY_EXE)
	@dy-path$(DANCY_EXE)

system:
	@mkdir "system"

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	dy-blob$(DANCY_EXE) -t loader $@

!include .\scripts\system.mk
!include .\scripts\tools.mk
