#!/bin/bash

INK='@INKSCAPE@'
ICONSET='@CMAKE_CURRENT_SOURCE_DIR@/pd_ceammc.iconset'
ICONSET_WIN='@CMAKE_CURRENT_SOURCE_DIR@/pd_ceammc_win.iconset'

if [ -z "$1" ]
then
	echo "SVG file needed."
	exit;
fi

mkdir -p ${ICONSET}
rm -rf ${ICONSET_WIN}
mkdir -p ${ICONSET_WIN}

BASE=${ICONSET}/icon
SVG="`pwd`/$1"

make_icon() {
    sz=$1
    sz2=$((${sz} * 2))
    $INK -z -D -e "${BASE}_${sz}x${sz}.png"    -f $SVG -w ${sz} -h ${sz}
    $INK -z -D -e "${BASE}_${sz}x${sz}@2x.png" -f $SVG -w ${sz2} -h ${sz2}
}

make_win_icon() {
    svg=$1
    sz=$2
    $INK -z -C -e "${ICONSET_WIN}/icon_${sz}x${sz}.png" -f `pwd`/$svg -w ${sz} -h ${sz}
}

make_icon 16
make_icon 32
make_icon 128
make_icon 256
make_icon 512
make_win_icon pd_ceammc_win_16.svg 16
make_win_icon pd_ceammc_win_32.svg 32
make_win_icon pd_ceammc_win_48.svg 48
make_win_icon pd_ceammc_win_64.svg 64
make_win_icon pd_ceammc_win_48.svg 96
make_win_icon pd_ceammc_win_64.svg 256

cp ${ICONSET}/icon_512x512.png ./puredata-ceammc.png
cp ${ICONSET_WIN}/icon_256x256.png ./puredata-ceammc_win.png

if [ -e "@ICONUTIL@" ]
then
    echo "Making ICNS..."
    @ICONUTIL@ -c icns "${ICONSET}"
fi


if [ -e "@MAGICK_CONVERT@" ]
then
    echo "Making ICO..."
    PNGS=$(find "${ICONSET}" -name '*.png' | grep -v '@')
    @MAGICK_CONVERT@ ${PNGS} pd_ceammc.ico

    PNGS=$(find "${ICONSET_WIN}" -name '*.png')
    @MAGICK_CONVERT@ ${PNGS} pd_ceammc_win.ico
fi
