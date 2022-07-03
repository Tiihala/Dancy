#!/bin/bash

if [ ! -f "scripts/clean.sh" ]; then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

rm -rf ./arctic/bin32
rm -rf ./arctic/bin64
rm -rf ./arctic/o32
rm -rf ./arctic/o64
rm -rf ./bin
rm -rf ./efi
rm -rf ./include/acpica
rm -rf ./kernel/acpica
rm -rf ./o32
rm -rf ./o64
rm -rf ./release
rm -rf ./system

rm -f ./scripts/dancy.mk
rm -f ./LOADER.512
rm -f ./LOADER.AT

rm -f `find . -name "*.obj"`
