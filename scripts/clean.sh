#!/bin/bash

if [ ! -f "scripts/clean.sh" ]; then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

rm -rf ./bin
rm -rf ./efi
rm -rf ./include/acpica
rm -rf ./kernel/acpica
rm -rf ./o32
rm -rf ./o64
rm -rf ./release

rm -f ./scripts/dancy.mk
rm -f ./system/*.AT
rm -f ./LOADER.512
rm -f ./LOADER.AT

rm -f `find -name "*.obj"`
