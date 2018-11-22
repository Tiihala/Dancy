#!/bin/bash
set -e

if [ ! -f "scripts/path.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

export DANCY_ROOT=`pwd`
export PATH="$DANCY_ROOT/bin:$DANCY_ROOT/external/bin:$PATH"
echo "$PATH"
