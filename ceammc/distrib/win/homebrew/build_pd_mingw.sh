#!/bin/bash

if [ $# -ne 1 ]
then
    echo "Usage $(basename $0) PATH_TO_PD..."
    exit 1
fi

PD_DIR=$1
CWD=`pwd`

PREFIX="/opt/local/mingw32"

export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig"
ls ${PKG_CONFIG_PATH}

HOST=i686-w64-mingw32
CC=${HOST}-gcc
CPP=${HOST}-cpp
CXX=${HOST}-c++
AR=${HOST}-ar

mkdir -p pd
cd pd
#rm -rf *

export ProgramW6432="${HOME}/.wine/drive_c/Program Files/"
export PROGRAMFILES="${HOME}/.wine/drive_c/Program Files/"
export DNSSD_ROOT="${HOME}/.wine/drive_c/Program\ Files/Bonjour SDK"

cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DBOOST_ROOT=/usr/local/include \
    -DLEAPMOTION_ROOT=/Users/serj/work/misc/LeapSDK_win/LeapSDK \
    -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32.cmake \
    -DWITH_DUMMY_AUDIO=OFF \
    -DWITH_DUMMY_MIDI=OFF \
    -DWITH_PORTAUDIO=ON \
    -DWITH_MMIO=OFF \
    -DCMAKE_INSTALL_PREFIX=${PREFIX}/pd \
    ${PD_DIR}

make -j3
make install
