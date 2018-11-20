# Dancy Operating System

include ./VERSION
include ./scripts/header.mk

HOST_CPPFLAGS=-I./include $(DANCY_VERSION)
HOST_CFLAGS=-O2 -std=c89 -Wall -Wextra -Wshadow -Wwrite-strings -pedantic

DANCY_EXE=
DANCY_HOST_BINARY=$(CC) -o
DANCY_HOST_OBJECT=$(CC) -c $(HOST_CPPFLAGS) $(HOST_CFLAGS) -o

all: $(DANCY_TARGET_ALL)

bin:
	mkdir bin

clean:
	@rm -rf bin
	@rm -rf o32
	@rm -rf o64
	@rm -rf system
	@rm -fv LOADER.*
	@rm -fv `find -name "*.obj"`

distclean: clean
	@rm -rf external

system:
	mkdir system

./LOADER.512: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t ldr512 $@

./LOADER.AT: ./bin/dy-blob$(DANCY_EXE)
	./bin/dy-blob$(DANCY_EXE) -t loader $@

include ./scripts/footer.mk
