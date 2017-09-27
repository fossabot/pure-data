#!/bin/bash

PKG=$1
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
cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=${PREFIX} \
    -DBOOST_ROOT=/usr/local/include \
    -DLEAPMOTION_ROOT=/Users/serj/work/misc/LeapSDK_win/LeapSDK \
    -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32.cmake \
    -DCMAKE_INSTALL_PREFIX=${PREFIX}/pd \
    ../../../music/pure-data/

make -j3
make install
