# Dancy Operating System

CPPFLAGS=-I./include
CFLAGS=-O2 -std=c89 -Wall -Wextra -pedantic

DANCY_EXE=
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(CPPFLAGS) $(CFLAGS) -o

include ./VERSION
include ./scripts/header.mk

all: $(DANCY_TARGET_ALL)

clean:
	$(RM) LOADER.*
	$(RM) `find bin -name "dy-*"`
	$(RM) `find -name "*.obj"`

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t loader $@

include ./scripts/footer.mk
