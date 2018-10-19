#!/bin/bash
set -e

BIN_VERSION=2.30
GCC_VERSION=7.3.0

BIN_LINK=ftp://ftp.gnu.org/gnu/binutils/binutils-$BIN_VERSION.tar.xz
GCC_LINK=ftp://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz

if [ ! -f "scripts/external/mingw.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

if [ -d "external/bin" ] || [ -d "external/src" ]
then
    echo "Error: remove previous tools first (make distclean)" 1>&2
    exit 1
fi

which make
which tar
which wget

export DANCY_EXTERNAL=`pwd`/external
mkdir -p $DANCY_EXTERNAL/bin
mkdir -p $DANCY_EXTERNAL/src

export PREFIX="$DANCY_EXTERNAL"
export PATH="$PREFIX/bin:$PATH"

pushd external/src
    wget $BIN_LINK
    wget $GCC_LINK
    tar -xf binutils-$BIN_VERSION.tar.xz
    tar -xf gcc-$GCC_VERSION.tar.xz
    rm binutils-$BIN_VERSION.tar.xz
    rm gcc-$GCC_VERSION.tar.xz
popd

pushd external/src/gcc-$GCC_VERSION
    contrib/download_prerequisites
popd

mkdir external/src/binutils-build
pushd external/src/binutils-build
    ../binutils-$BIN_VERSION/configure \
        --prefix=$PREFIX \
        --target=x86_64-w64-mingw32 \
        --enable-targets=x86_64-w64-mingw32,i686-w64-mingw32 \
        --disable-nls
    make
    make install
popd

mkdir external/src/gcc-build
pushd external/src/gcc-build
    which -- x86_64-w64-mingw32-as
    ../gcc-$GCC_VERSION/configure \
        --prefix=$PREFIX \
        --target=x86_64-w64-mingw32 \
        --enable-targets=all \
        --disable-nls \
        --without-headers \
        --enable-languages=c
    make all-gcc
    make install-gcc
popd

rm -rf external/src
echo "scripts/external/mingw.sh: done"
