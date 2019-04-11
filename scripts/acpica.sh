#!/bin/bash
set -e

ACPICA_TAG=R02_15_19
ACPICA_GIT=https://github.com/acpica/acpica.git

if [ ! -f "scripts/acpica.sh" ]; then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

mkdir -p external
mkdir -p external/tmp
mkdir -p include/acpica
mkdir -p include/acpica/platform
mkdir -p kernel/acpica

if [ ! -d "external/tmp/acpica" ]; then
    git clone $ACPICA_GIT external/tmp/acpica
    git -C external/tmp/acpica checkout $ACPICA_TAG
    rm -rf external/tmp/acpica/.git
    rm -rf external/tmp/acpica/generate
    rm -rf external/tmp/acpica/tests
    rm -f external/tmp/acpica/.gitignore
    rm -f external/tmp/acpica/Makefile
    bin/dy-patch -p1 -i kernel/acpios/patches/acenv
    bin/dy-patch -p1 -i kernel/acpios/patches/acenvex
    bin/dy-patch -p1 -i kernel/acpios/patches/rsdump
fi

ACPICA_EXT=.${1##*.}
ACPICA_FILE=${1##*/}

ACPICA_H=external/tmp/acpica/source/include
ACPICA_C=external/tmp/acpica/source/components

if [ "$ACPICA_EXT" = ".h" ]; then
    ACPICA_SOURCE=`find "$ACPICA_H" | grep "$ACPICA_FILE"`
fi

if [ "$ACPICA_EXT" = ".c" ]; then
    ACPICA_SOURCE=`find "$ACPICA_C" | grep "$ACPICA_FILE"`
fi

if [ ! -z "$ACPICA_SOURCE" ]; then
    cat "$ACPICA_SOURCE" > "${1}"
fi
