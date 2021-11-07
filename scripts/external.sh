#!/bin/bash
set -e

ASM_VERSION=2.14.02
BIN_VERSION=2.30
GCC_VERSION=7.3.0

ASM_ADDR=https://www.nasm.us/pub/nasm
GNU_ADDR=ftp://ftp.gnu.org/gnu

ASM_LINK=$ASM_ADDR/releasebuilds/$ASM_VERSION/nasm-$ASM_VERSION.tar.xz
BIN_LINK=$GNU_ADDR/binutils/binutils-$BIN_VERSION.tar.xz
GCC_LINK=$GNU_ADDR/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz

if [ ! -f "scripts/external.sh" ]
then
    echo "Error: does not look like a root of Dancy source tree" 1>&2
    exit 1
fi

if [ -d "external/bin" ] || [ -d "external/src" ]
then
    echo "Error: remove previous tools first (make distclean)" 1>&2
    exit 1
fi

export DANCY_EXTERNAL=`pwd`/external
export PREFIX="$DANCY_EXTERNAL"
export PATH="$PREFIX/bin:$PATH"

mkdir -p external/bin
mkdir -p external/src

ASM_AVAILABLE=0
GCC_AVAILABLE=0

if [ -f "/usr/bin/nasm" ]
then
    ASM_AVAILABLE=1
fi

if [ -f "/usr/bin/yasm" ]
then
    ASM_AVAILABLE=1
fi

if [ -f "/usr/bin/x86_64-pc-msys-gcc.exe" ]
then
    # Assume that ASM and GCC both are available
    ASM_AVAILABLE=1
    GCC_AVAILABLE=1
fi

if [ -f "/usr/bin/x86_64-w64-mingw32-gcc" ]
then
    GCC_AVAILABLE=1
fi

if [ $ASM_AVAILABLE -eq 0 ]
then
    echo -e "\e[33mBuilding an assembler:"
    echo -e "\e[1m    nasm-$ASM_VERSION"
    echo -e "\e[0m"
fi

if [ $GCC_AVAILABLE -eq 0 ]
then
    echo -e "\e[33mBuilding a cross compiler:"
    echo -e "\e[1m    binutils-$BIN_VERSION"
    echo -e "\e[1m    gcc-$GCC_VERSION"
    echo -e "\e[0m"
fi

if [ $ASM_AVAILABLE -eq 0 ] || [ $GCC_AVAILABLE -eq 0 ]
then
    echo -e "\e[33mDownloading and building external code may take some time!"
    echo -e "All files are put in the \e[34m./external\e[33m directory."
    echo -e "\e[0m"
    sleep 5

    which gcc
    which g++
    which make
    which tar
    which wget
fi

if [ $ASM_AVAILABLE -eq 0 ]
then
pushd external/src
    wget $ASM_LINK
    tar -xf nasm-$ASM_VERSION.tar.xz
popd

pushd external/src/nasm-$ASM_VERSION
    ./configure --prefix=$PREFIX
    make
    make install
popd
fi

if [ $GCC_AVAILABLE -eq 0 ]
then
pushd external/src
    wget $BIN_LINK
    wget $GCC_LINK
    tar -xf binutils-$BIN_VERSION.tar.xz
    tar -xf gcc-$GCC_VERSION.tar.xz
popd

mkdir -p external/mingw/include

pushd external/mingw/include
    touch limits.h
    touch stdarg.h
    touch stddef.h
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
        --disable-nls \
        --with-sysroot=$PREFIX
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
        --enable-languages=c \
        --without-headers \
        --disable-nls \
        --with-sysroot=$PREFIX
    make all-gcc
    make install-gcc
popd
fi

rm -rf external/src/*
touch external/external.sh
