#!/bin/bash
set -e

ACPICA_TAG=R03_31_21
ACPICA_GIT=https://github.com/acpica/acpica.git

if [ ! -f "scripts/git/acpica.sh" ]; then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

mkdir -p external
mkdir -p common/include/acpica
mkdir -p common/include/acpica/platform
mkdir -p kernel/acpica

if [ ! -d "external/acpica" ]; then
    git clone $ACPICA_GIT external/acpica
    git -C external/acpica checkout $ACPICA_TAG
    bin/dy-patch -p1 -i kernel/acpios/patches/acenv
    bin/dy-patch -p1 -i kernel/acpios/patches/acenvex
    bin/dy-patch -p1 -i kernel/acpios/patches/rsdump
fi

ACPICA_EXT=.${1##*.}
ACPICA_FILE=${1##*/}

ACPICA_H=external/acpica/source/include
ACPICA_C=external/acpica/source/components

if [ "$ACPICA_EXT" = ".h" ]; then
    ACPICA_SOURCE=`find "$ACPICA_H" | grep "$ACPICA_FILE"`
fi

if [ "$ACPICA_EXT" = ".c" ]; then
    ACPICA_SOURCE=`find "$ACPICA_C" | grep "$ACPICA_FILE"`
fi

if [ ! -z "$ACPICA_SOURCE" ]; then
    cat "$ACPICA_SOURCE" > "${1}"
fi
