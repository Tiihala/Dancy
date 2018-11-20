# Dancy Operating System

HOST_CPPFLAGS=-I./include
HOST_CFLAGS=/O2 /Wall /wd4711 /wd4996 /nologo

DANCY_EXE=.exe
DANCY_HOST_BINARY=CL /Fe
DANCY_HOST_OBJECT=CL /c $(HOST_CPPFLAGS) $(HOST_CFLAGS) /Fo

!include .\VERSION
!include .\scripts\header.mk

all: $(DANCY_TARGET_ALL)

bin:
	mkdir "bin"

clean:
	@if exist bin rmdir bin /S /Q
	@if exist o32 rmdir o32 /S /Q
	@if exist o64 rmdir o64 /S /Q
	@if exist system rmdir system /S /Q
	@for %i in (LOADER.*) do del "%i"
	@for /R %i in (*.obj) do del "%i"

distclean: clean
	@if exist external rmdir external /S /Q

system:
	mkdir "system"

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	bin\dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	bin\dy-blob$(DANCY_EXE) -t loader $@

!include .\scripts\footer.mk
