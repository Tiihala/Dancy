#!/bin/bash
set -e

if [ ! -f "scripts/gcc.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

export DANCY_ROOT=`pwd`
export PATH="$DANCY_ROOT/bin:$DANCY_ROOT/external/bin:$PATH"

COMPILER_BIN=`which x86_64-w64-mingw32-gcc || true`

if [ -n "$COMPILER_BIN" ]
then
    x86_64-w64-mingw32-gcc "$@"
    exit 0
fi

COMPILER_BIN=`which x86_64-pc-msys-gcc.exe || true`

if [ -n "$COMPILER_BIN" ]
then
    x86_64-pc-msys-gcc.exe "$@"
    exit 0
fi

echo "Error: compiler is not found" 1>&2
exit 1
