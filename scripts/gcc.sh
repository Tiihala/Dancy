#!/bin/bash
set -e

if [ ! -f "scripts/gcc.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

if [ -f "./external/bin/x86_64-w64-mingw32-gcc" ]
then
    echo "./external/bin/x86_64-w64-mingw32-gcc"
    exit 0
fi

if [ -f "/usr/bin/x86_64-pc-msys-gcc.exe" ]
then
    echo "x86_64-pc-msys-gcc"
    exit 0
fi

if [ -f "/usr/bin/x86_64-w64-mingw32-gcc" ]
then
    echo "/usr/bin/x86_64-w64-mingw32-gcc"
    exit 0
fi

echo "Error: compiler is not found" 1>&2
exit 1
