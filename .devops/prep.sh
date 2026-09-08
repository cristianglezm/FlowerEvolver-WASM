#!/bin/bash
#
# Dev environment setup. Three modes:
#   bash .devops/prep.sh wasm    (default) - emsdk + Emscripten-built
#                                      JsonBox/EvoAI, for the WASM/npm build.
#                                      Kept under build/wasm/, sibling to
#                                      build/native/ and build/android/ below
#                                      - same subfolder-per-mode convention
#                                      for all three, so a developer can have
#                                      a native build, a wasm build, and an
#                                      android build set up side by side
#                                      without any of them clobbering another.
#   bash .devops/prep.sh native  - plain JsonBox/EvoAI built with
#                                      -DCMAKE_POSITION_INDEPENDENT_CODE=ON,
#                                      for the native static/shared CMake
#                                      targets and their tests.
#   bash .devops/prep.sh android [abi...]  - JsonBox/EvoAI cross-compiled
#                                      for Android via CMake's own native
#                                      Android support (CMAKE_SYSTEM_NAME=
#                                      Android).
#                                      Needs $ANDROID_NDK pointing at
#                                      a real NDK install; this script
#                                      doesn't install one. Defaults to all
#                                      four Play-Store-relevant ABIs if none
#                                      are given; pass your own to build a
#                                      subset, e.g.:
#                                        bash .devops/prep.sh android arm64-v8a
#                                      JsonBox's and EvoAI's own CMakeLists.txt
#                                      force-install to
#                                      $ANDROID_NDK/sources/third_party/<name>
#                                      whenever cross-compiling for Android,
#                                      with a lib/<abi>/ split, so all four
#                                      ABIs correctly coexist under that one
#                                      shared, NDK-relative location (not
#                                      this repo's own build/ directory,
#                                      see the comment on the android case
#                                      below for what that implies locally).
#
# Safe to run more than once for any mode - existing clones are reused and
# CMake/Ninja are naturally incremental, so re-running only redoes work
# that's actually needed.
#
# Run `npm run prep` yourself when you actually want the wasm
# toolchain set up; `npm run prep:native` / `npm run prep:android [abi...]`
# for those builds instead.
#
# Assumes it is run from the repository root (same assumption package.json
# scripts and the other .devops/*.sh scripts already make).

set -e

MODE="${1:-wasm}"
if [ $# -gt 0 ]; then
    shift
fi
EMSDK_VERSION=6.0.6

mkdir -p build
cd build

case "$MODE" in
    wasm)
        # kept under build/wasm/ - sibling to build/native/ and
        # build/android/ below, same convention for all three modes now
        mkdir -p wasm
        cd wasm

        if [ ! -d emsdk ]; then
            git clone https://github.com/emscripten-core/emsdk.git
        fi
        cd emsdk
        ./emsdk install $EMSDK_VERSION
        ./emsdk activate $EMSDK_VERSION
        source ./emsdk_env.sh
        cd ..

        if [ ! -d JsonBox ]; then
            git clone https://github.com/cristianglezm/JsonBox.git
        fi
        mkdir -p JsonBox/build
        cd JsonBox/build
        emcmake cmake -G Ninja -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
        cmake --build . -j 4 --target install
        cd ../..

        if [ ! -d EvoAI ]; then
            git clone https://github.com/cristianglezm/EvoAI.git
        fi
        mkdir -p EvoAI/build
        cd EvoAI/build
        emcmake cmake -G Ninja \
            -DEvoAI_BUILD_STATIC=TRUE \
            -DJsonBox_ROOT=../../JsonBox/build/install \
            -DCMAKE_INSTALL_PREFIX="$(pwd)/install" \
            ..
        cmake --build . -j 4 --target install
        cd ../..

        cd ..
        echo "prep.sh (wasm) done: emsdk activated, JsonBox + EvoAI installed under build/wasm/"
        ;;
    native)
        # kept under build/native/ - these are compiled with a different
        # toolchain/flags than the wasm copies above, so the two sets of
        # build artifacts can't share a directory.
        mkdir -p native
        cd native

        if [ ! -d JsonBox ]; then
            git clone https://github.com/cristianglezm/JsonBox.git
        fi
        mkdir -p JsonBox/build
        cd JsonBox/build
        cmake -G Ninja \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            -DCMAKE_INSTALL_PREFIX="$(pwd)/install" \
            -DCMAKE_BUILD_TYPE=Release \
            ..
        cmake --build . -j 4 --target install
        cd ../..

        if [ ! -d EvoAI ]; then
            git clone https://github.com/cristianglezm/EvoAI.git
        fi
        mkdir -p EvoAI/build
        cd EvoAI/build
        cmake -G Ninja \
            -DEvoAI_BUILD_STATIC=TRUE \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            -DJsonBox_ROOT=../../JsonBox/build/install \
            -DCMAKE_INSTALL_PREFIX="$(pwd)/install" \
            -DCMAKE_BUILD_TYPE=Release \
            ..
        cmake --build . -j 4 --target install
        cd ../..

        cd ..
        echo "prep.sh (native) done: JsonBox + EvoAI installed under build/native/"
        ;;
    android)
        if [ -z "$ANDROID_NDK" ]; then
            echo "error: \$ANDROID_NDK is not set -- point it at your Android NDK install (e.g. \$ANDROID_SDK_ROOT/ndk/<version> on GitHub's own Android runner images)." >&2
            exit 1
        fi
        ANDROID_API="${ANDROID_API:-33}"
        ABIS=("$@")
        if [ ${#ABIS[@]} -eq 0 ]; then
            ABIS=(armeabi-v7a arm64-v8a x86 x86_64)
        fi

        mkdir -p android
        cd android

        if [ ! -d JsonBox ]; then
            git clone https://github.com/cristianglezm/JsonBox.git
        fi
        if [ ! -d EvoAI ]; then
            git clone https://github.com/cristianglezm/EvoAI.git
        fi
        ANDROID_DEPS_ROOT="$ANDROID_NDK/sources/third_party"
        for abi in "${ABIS[@]}"; do
            echo "--- JsonBox ($abi, API $ANDROID_API) ---"
            cmake -S JsonBox -B "JsonBox/build-$abi" \
                -G Ninja \
                -DCMAKE_BUILD_TYPE=Release \
                -DCMAKE_SYSTEM_NAME=Android \
                -DCMAKE_ANDROID_NDK="$ANDROID_NDK" \
                -DCMAKE_ANDROID_ARCH_ABI="$abi" \
                -DCMAKE_ANDROID_API="$ANDROID_API" \
                -DCMAKE_ANDROID_STL_TYPE=c++_shared \
                -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
                -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
                -DCMAKE_INSTALL_PREFIX="$ANDROID_DEPS_ROOT/JsonBox"
            cmake --build "JsonBox/build-$abi" -j 4 --target install

            echo "--- EvoAI ($abi, API $ANDROID_API) ---"
            cmake -S EvoAI -B "EvoAI/build-$abi" \
                -G Ninja \
                -DCMAKE_BUILD_TYPE=Release \
                -DEvoAI_BUILD_STATIC=TRUE \
                -DCMAKE_SYSTEM_NAME=Android \
                -DCMAKE_ANDROID_NDK="$ANDROID_NDK" \
                -DCMAKE_ANDROID_ARCH_ABI="$abi" \
                -DCMAKE_ANDROID_API="$ANDROID_API" \
                -DCMAKE_ANDROID_STL_TYPE=c++_shared \
                -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
                -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
                -DJsonBox_ROOT="$ANDROID_DEPS_ROOT/JsonBox" \
                -DCMAKE_INSTALL_PREFIX="$ANDROID_DEPS_ROOT/EvoAI"
            cmake --build "EvoAI/build-$abi" -j 4 --target install
        done

        cd ..
        echo "prep.sh (android) done: JsonBox + EvoAI installed under \$ANDROID_NDK/sources/third_party/{JsonBox,EvoAI}/lib/<abi>/ for: ${ABIS[*]}"
        ;;
    *)
        echo "prep.sh: unknown mode '$MODE' (expected 'wasm', 'native', or 'android')" >&2
        exit 1
        ;;
esac
cd ..
