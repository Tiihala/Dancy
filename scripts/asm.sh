#!/bin/bash
set -e

if [ ! -f "scripts/asm.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

export DANCY_ROOT=`pwd`
export PATH="$DANCY_ROOT/bin:$DANCY_ROOT/external/bin:$PATH"

ASSEMBLER_BIN=`which nasm || true`

if [ -n "$ASSEMBLER_BIN" ]
then
    nasm "$@"
    exit 0
fi

ASSEMBLER_BIN=`which yasm || true`

if [ -n "$ASSEMBLER_BIN" ]
then
    yasm "$@"
    exit 0
fi

echo "Error: assembler is not found" 1>&2
exit 1
