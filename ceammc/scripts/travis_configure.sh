#/bin/bash

uname -a
printenv

if [[ ${CEAMMC_COVERAGE} == "ON" ]]
then
	gcov --version
fi

mkdir ${CEAMMC_BUILD_DIR} 
cd ${CEAMMC_BUILD_DIR}

cmake -DCMAKE_BUILD_TYPE=${CEAMMC_BUILD_TYPE} \
	-DWITH_ALSA_MIDI=ON \
	-DWITH_PORTMIDI=ON \
	-DWITH_PORTAUDIO=OFF \
	-DWITH_COVERAGE=${CEAMMC_COVERAGE} \
    -DCMAKE_INSTALL_PREFIX=/usr \
	..

