#/bin/bash

set -e

mkdir build && cd build
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh
cd ..
git clone https://github.com/cristianglezm/JsonBox.git
cd JsonBox && mkdir build && cd build
emcmake cmake -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
make -j4 install
cd ..
git clone https://github.com/cristianglezm/EvoAI.git
cd EvoAI && git checkout refactor-activations
mkdir build && cd build
emcmake -DEvoAI_BUILD_STATIC=TRUE -DJsonBox_ROOT=../JsonBox/build/install -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
make -j4 install
cd ..
cmake -DEvoAI_ROOT=EvoAI/build/install -DJsonBox_ROOT=JsonBox/build/install ..
make -j4 dist
cd ..