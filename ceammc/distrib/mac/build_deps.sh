#!/bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $(basename $0) package"
    exit 1
fi

PKG=$1
CWD=`pwd`

PREFIX=/opt/local/universal
export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig"
export CFLAGS='-arch x86_64 -arch i386 -O2'

function banner() {
    echo "*****************************************************"
    tput setaf 2
    echo $1
    tput sgr0
    echo "*****************************************************"
}

function install_libmodplug() {
    pkg="libmodplug"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf libmodplug*

    banner "Downloading ${pkg}"
    wget https://codeload.github.com/Konstanty/libmodplug/zip/master -O libmodplug.zip
    unzip -o libmodplug.zip
    cd libmodplug-*

    banner "Configure ${pkg}"
    autoreconf -fi
    export CXXFLAGS='-arch x86_64 -arch i386 -O2'
    ./configure --enable-static=no \
        --enable-shared=yes \
        --prefix=${PREFIX}

    banner "Build ${pkg}"
    make

    banner "Installing ${pkg}"
    make install
}

function install_sndfile() {
    pkg="sndfile"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf libsndfile*

    banner "Downloading ${pkg}"
    brew unpack libsndfile
    cd libsndfile*

    banner "Configure ${pkg}"
    CFLAGS="${CFLAGS} -I${PREFIX}/include" ./configure --enable-static=yes \
        --enable-shared=yes \
        --prefix=${PREFIX}

    banner "Build ${pkg}"
    make

    banner "Installing ${pkg}"
    make install
}

function install_ogg() {
    pkg="OGG"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf libogg*

    banner "Downloading ${pkg}"
    brew unpack libogg
    cd libogg*

    banner "Configure ${pkg}"
    ./configure --enable-static=yes \
        --enable-shared=yes \
        --prefix=${PREFIX}

    banner "Build ${pkg}"
    make

    banner "Installing ${pkg}"
    make install
}

function install_portaudio() {
    pkg="Portaudio"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf portaudio*

    banner "Downloading ${pkg}"
    brew unpack portaudio

    cd portaudio*

    ./configure --prefix=${PREFIX} \
        --enable-static=no \
        --enable-shared=yes

    make
    make install
}

function install_vorbis() {
    pkg="vorbis"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf libvorbis*

    banner "Downloading ${pkg}"
    brew unpack libvorbis
    cd libvorbis*

    banner "Configure ${pkg}"
    ./configure --enable-static=yes \
        --enable-shared=yes \
        --prefix=${PREFIX}

    banner "Build ${pkg}"
    make

    banner "Installing ${pkg}"
    make install
}

function install_flac() {
    pkg="FLAC"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf flac*

    banner "Downloading ${pkg}"
    brew unpack flac
    cd flac*

    banner "Configure ${pkg}"
    ./configure --enable-static=no \
        --enable-shared=yes \
        --disable-asm-optimizations \
        --disable-dependency-tracking \
        --disable-cpplibs \
        --prefix=${PREFIX}

    banner "Build ${pkg}"
    make

    banner "Installing ${pkg}"
    make install
}

function install_fftw3() {
    pkg="FFTW3"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf fftw*

    banner "Downloading ${pkg}"
    brew unpack fftw
    cd fftw-*

    banner "Configure ${pkg} float"
    ./configure --enable-single \
        --prefix=${PREFIX} \
        --disable-fortran \
        --enable-sse \
        --enable-sse2 \
        --enable-avx \
        --with-our-malloc \
        --enable-shared=yes \
        --enable-static=no

    banner "Build ${pkg} float"
    make -j3

    banner "Install ${pkg} float"
    make install
}

function install_tcl() {
    export CFLAGS='-O2 -arch x86_64'
    pkg="TCL"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf tcl-release

    banner "Downloading ${pkg}"
    if [ ! -f "tcl-release.zip" ]
    then
        wget "https://github.com/tcltk/tcl/archive/release.zip" -O "tcl-release.zip"
    fi

    unzip "tcl-release.zip"
    cd tcl-release/macosx

    banner "Build ${pkg}"
    make -f GNUMakefile

    banner "Installing ${pkg}"
    sudo make install
}

function install_tcllib() {
    pkg="tcllib"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf tcllib-*

    banner "Downloading ${pkg}"
    if [ ! -f "tcllib.zip" ]
    then
        wget "https://github.com/tcltk/tcllib/archive/tcllib_1_18.zip" -O "tcllib.zip"
    fi

    unzip "tcllib.zip"
    cd tcllib-*

    banner "Configure ${pkg}"
    ./configure  --with-tclsh="/usr/local/bin/tclsh8.6" \
        --prefix=${PREFIX}

    banner "Build ${pkg}"
    make

    banner "Installing ${pkg}"
    make install
}

function install_tklib() {
    pkg="tklib"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf tklib-*

    banner "Downloading ${pkg}"
    if [ ! -f "tklib.zip" ]
    then
        wget "https://codeload.github.com/tcltk/tklib/zip/master" -O "tklib.zip"
    fi

    unzip "tklib.zip"
    LIBNAME=`basename tklib-*`
    cd tklib-*

    banner "Configure ${pkg}"
    ./configure --prefix=${PREFIX}

    banner "Build ${pkg}"
    banner "Installing ${pkg}"
    make install
}


function install_tk() {
    export CFLAGS='-O2 -arch x86_64'
    pkg="TK"
    cd "${CWD}"

    banner "${pkg}"
    rm -rf tk-release

    banner "Downloading ${pkg}"
    if [ ! -f "tk-release.zip" ]
    then
        wget "https://github.com/tcltk/tk/archive/release.zip" -O tk-release.zip
    fi

    unzip "tk-release.zip"
    cd tk-release/macosx

    banner "Configure ${pkg}"
    banner "Build ${pkg}"
    make -f GNUMakefile

    banner "Installing ${pkg}"
    sudo make install
}

case ${PKG} in
    all)
        install_libmodplug
        install_fftw3
        install_tcl
        install_tcllib
        install_tk
        install_tklib
        install_ogg
        install_vorbis
        install_flac
        install_sndfile
        install_portaudio
        ;;
    modplug)
        install_libmodplug
        ;;
    fftw3)
        install_fftw3
        ;;
    tcl)
        install_tcl
        ;;
    tcllib)
        install_tcllib
        ;;
    tk)
        install_tk
        ;;
    tklib)
        install_tklib
        ;;
    ogg)
        install_ogg
        ;;
    vorbis)
        install_vorbis
        ;;
    flac)
        install_flac
        ;;
    sndfile)
        install_sndfile
        ;;
    portaudio)
        install_portaudio
        ;;
    *)
        echo "Choose from following: modplug, fftw3, tcl, tcllib, tk, tklib, ogg, \
              vorbis, flac, portaudio or all"
        exit 1
        ;;
esac
