# Dancy Operating System

CPPFLAGS=-I./include
CFLAGS=/O2 /Wall /wd4996 /nologo

DANCY_EXE=.exe
DANCY_HOST_BINARY=CL /Fe
DANCY_HOST_OBJECT=CL /c $(CPPFLAGS) $(CFLAGS) /Fo

!include .\VERSION
!include .\scripts\header.mk

all: $(DANCY_TARGET_ALL)

clean:
	@for %i in (LOADER.*) do del "%i"
	@for /R %i in (dy-*.exe) do del "%i"
	@for /R %i in (*.obj) do del "%i"

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	bin\dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	bin\dy-blob$(DANCY_EXE) -t loader $@

!include .\scripts\footer.mk
