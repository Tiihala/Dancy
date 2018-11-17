# Dancy Operating System

DANCY_TARGET_ALL= \
 ./LOADER.512 \
 ./LOADER.AT \
 ./bin/dy-blob$(DANCY_EXE) \
 ./bin/dy-gpt$(DANCY_EXE) \
 ./bin/dy-init$(DANCY_EXE) \
 ./bin/dy-link$(DANCY_EXE) \
 ./bin/dy-mbr$(DANCY_EXE) \
 ./bin/dy-vbr$(DANCY_EXE)

DANCY_HEADERS= \
 ./include/dancy/limits.h \
 ./include/dancy.h
