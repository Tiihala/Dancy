#!/bin/bash
set -e

if [ ! -f "scripts/asm.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

if [ -f "./external/bin/nasm" ]
then
    echo "./external/bin/nasm"
    exit 0
fi

if [ -f "/usr/bin/nasm" ]
then
    echo "/usr/bin/nasm"
    exit 0
fi

echo "Error: assembler is not found" 1>&2
exit 1
