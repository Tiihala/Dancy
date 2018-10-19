# Dancy Operating System

HOST_CPPFLAGS=-I./include
HOST_CFLAGS=-O2 -std=c89 -Wall -Wextra -Wwrite-strings -pedantic

DANCY_EXE=
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

include ./VERSION
include ./scripts/header.mk

all: $(DANCY_TARGET_ALL)

clean:
	@rm -fv LOADER.*
	@rm -fv `find bin -name "dy-*"`
	@rm -fv `find -name "*.o"`
	@rm -fv `find -name "*.obj"`

distclean: clean
	@rm -rf external

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t loader $@

include ./scripts/footer.mk
