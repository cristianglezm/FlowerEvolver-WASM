#/bin/bash

set -e

FE_EXPORT=$1

mkdir build && cd build
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
# we need to install tsc for FE_EXPORT=TRUE
cd upstream/emscripten && npm i tsc
cd ../../
source emsdk_env.sh
cd ..
git clone https://github.com/cristianglezm/JsonBox.git
cd JsonBox && mkdir build && cd build
emcmake cmake -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
make -j4 install
cd ../..
git clone https://github.com/cristianglezm/EvoAI.git
cd EvoAI
mkdir build && cd build
emcmake cmake -DEvoAI_BUILD_STATIC=TRUE -DJsonBox_ROOT=../../JsonBox/build/install -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
make -j4 install
cd ../..
emcmake cmake -DFE_EXPORT=$FE_EXPORT -DEvoAI_ROOT="$(pwd)/EvoAI/build/install" -DJsonBox_ROOT="$(pwd)/JsonBox/build/install" ..
make -j4 dist
cd ..
