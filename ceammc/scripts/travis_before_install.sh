#!/bin/bash

sudo apt-get install -qq libboost-dev libsndfile1 libsndfile-dev\
	libportaudio-dev \
	libfftw3-dev libasound2-dev libjack-dev \
	libmodplug-dev libavahi-compat-libdnssd-dev

if [[ ${CEAMMC_COVERAGE} == "ON" ]]
then
	echo "Installing cpp-coveralls ..."
	pip install --user cpp-coveralls
fi

