#!/bin/bash
set -e

if [ ! -f "scripts/asm.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

export DANCY_ROOT=`pwd`
export PATH="$DANCY_ROOT/bin:$DANCY_ROOT/external/bin:$PATH"

if
    which nasm > /dev/null 2>&1
then
    nasm "$@"
    exit 0
fi

echo "Error: a compatible assembler was not found" 1>&2
exit 1
