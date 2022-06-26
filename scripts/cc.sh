#!/bin/bash
set -e

if [ ! -f "scripts/cc.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

export DANCY_ROOT=`pwd`
export PATH="$DANCY_ROOT/bin:$DANCY_ROOT/external/bin:$PATH"

if
    which x86_64-w64-mingw32-gcc > /dev/null 2>&1
then
    x86_64-w64-mingw32-gcc "$@"
    exit 0
fi

if
    which x86_64-pc-msys-gcc.exe > /dev/null 2>&1
then
    x86_64-pc-msys-gcc.exe "$@"
    exit 0
fi

echo "Error: a compatible compiler was not found" 1>&2
exit 1
