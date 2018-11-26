# Dancy Operating System

!include .\VERSION
!include .\scripts\header.mk
DANCY_PATH=$(MAKEDIR)\bin;$(PATH)

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=/O2 /Wall /wd4711 /wd4996 /nologo

DANCY_EXE=.exe
DANCY_HOST_BINARY=CL /Fe
DANCY_HOST_OBJECT=CL /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

DANCY_AS=NASM.EXE
DANCY_CC=CLANG.EXE
DANCY_OBJECT_32=$(DANCY_CC) $(DANCY_CPPFLAGS_32) $(DANCY_CFLAGS_32) -c -m32 -o
DANCY_OBJECT_64=$(DANCY_CC) $(DANCY_CPPFLAGS_64) $(DANCY_CFLAGS_64) -c -m64 -o

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
	@cmd /C "set PATH=$(DANCY_PATH) && bin\dy-path$(DANCY_EXE)"

!include .\scripts\system.mk

./system/IN_IA32.AT: $(DANCY_INIT_OBJECTS_32)
	bin\dy-link$(DANCY_EXE) -o$@ -finit $(DANCY_INIT_OBJECTS_32)
	bin\dy-init$(DANCY_EXE) -tia32 --set-header $@

./system/IN_X64.AT: $(DANCY_INIT_OBJECTS_64)
	bin\dy-link$(DANCY_EXE) -o$@ -finit $(DANCY_INIT_OBJECTS_64)
	bin\dy-init$(DANCY_EXE) -tx64 --set-header $@

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	bin\dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	bin\dy-blob$(DANCY_EXE) -t loader $@

!include .\scripts\tools.mk
