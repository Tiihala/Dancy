#!/bin/bash
set -e

SORTIX_TAG=b99f6211411689645e5cdf21d99d5f308560e9c8
SORTIX_GIT=https://gitlab.com/sortix/sortix.git

if [ ! -f "scripts/git/sortix.sh" ]; then exit 1; fi

if [ ! -d "external/sortix" ]; then
    mkdir -p "external"
    git clone $SORTIX_GIT "external/sortix"
    git -C "external/sortix" checkout $SORTIX_TAG
fi
