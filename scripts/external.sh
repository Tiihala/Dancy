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

if [ -f "/usr/bin/x86_64-pc-msys-gcc.exe" ]
then
    mkdir -p $DANCY_EXTERNAL/bin
    touch external/external.sh
    exit 0
fi

echo ""
echo -e "\e[33mBuilding a cross compiler:\e[0m"
echo -e "\e[33m    nasm-$ASM_VERSION\e[0m"
echo -e "\e[33m    binutils-$BIN_VERSION\e[0m"
echo -e "\e[33m    gcc-$GCC_VERSION\e[0m"
echo ""
sleep 5

which gcc
which make
which tar
which wget

mkdir -p $DANCY_EXTERNAL/bin
mkdir -p $DANCY_EXTERNAL/mingw/include
mkdir -p $DANCY_EXTERNAL/src

export PREFIX="$DANCY_EXTERNAL"
export PATH="$PREFIX/bin:$PATH"

pushd external/mingw/include
    touch limits.h
    touch stdarg.h
    touch stddef.h
popd

pushd external/src
    wget $ASM_LINK
    wget $BIN_LINK
    wget $GCC_LINK
    tar -xf nasm-$ASM_VERSION.tar.xz
    tar -xf binutils-$BIN_VERSION.tar.xz
    tar -xf gcc-$GCC_VERSION.tar.xz
    rm nasm-$ASM_VERSION.tar.xz
    rm binutils-$BIN_VERSION.tar.xz
    rm gcc-$GCC_VERSION.tar.xz
popd

pushd external/src/nasm-$ASM_VERSION
    ./configure --prefix=$PREFIX
    make
    make install
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

rm -rf external/src
touch external/external.sh
echo -e "scripts/external.sh: \e[32mREADY\e[0m"
echo ""
