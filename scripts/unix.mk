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
	$(RM) LOADER.512
	$(RM) `find bin -name "dy-*"`
	$(RM) `find -name "*.obj"`

./LOADER.512: ./bin/dy-vbr$(DANCY_EXE)
	./bin/dy-vbr$(DANCY_EXE) -o$@

include ./scripts/footer.mk
