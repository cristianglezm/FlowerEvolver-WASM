#!/bin/bash
#
# Builds FlowerEvolver. Six modes:
#   bash .devops/build.sh wasm    (default) - configures + builds the
#                                    Emscripten/WASM module + demo dist/,
#                                    against the toolchain `npm run prep`
#                                    sets up under build/wasm/.
#   bash .devops/build.sh native  - configures + builds the native
#                                    static/shared library (add
#                                    -DFlowerEvolver_BUILD_TESTS=ON to also
#                                    build the Catch2 suite), against the
#                                    deps `npm run prep:native` sets up
#                                    under build/native/.
#   bash .devops/build.sh android <abi>  - configures + builds for a single
#                                    Android ABI (e.g. arm64-v8a), against
#                                    the deps `npm run prep:android` sets
#                                    up. The ABI is a required argument
#                                    (unlike wasm/native, there's no single
#                                    sensible default - you're building for
#                                    one specific device architecture, not
#                                    "whatever's around"). Needs $ANDROID_NDK
#                                    set, same as prep.sh's android mode.
#   bash .devops/build.sh docs    - builds the C++ (Doxygen+Breathe+
#                                    Sphinx) and JS/WASM (TypeDoc) API doc
#                                    sites and copies them into
#                                    dist/docs/{cpp,js}/, alongside whatever
#                                    the wasm mode has put in dist/ (in
#                                    either order -- this doesn't require
#                                    dist/ to already have the demo in it,
#                                    just creates dist/docs/ itself if
#                                    needed). Needs the docs toolchain
#                                    already installed: doxygen + graphviz
#                                    (system packages) and
#                                    `pip install -r docs/requirements.txt`
#                                    - this script doesn't install those
#                                    itself, same division of labor as
#                                    prep.sh handling C++ deps vs. this
#                                    script doing the actual build.
#                                    Also builds a third, Python (Sphinx +
#                                    autodoc) site into dist/docs/python/,
#                                    but only if FlowerEvolver is already
#                                    importable (`npm run prep:native` once,
#                                    then `npm run build:python`) -- see the
#                                    docs case below for why that one's a
#                                    soft skip rather than a hard
#                                    prerequisite of this whole mode.
#                                    Each site's actual build command also
#                                    runs standalone, for iterating on just
#                                    one: `npm run build:docs:cpp`,
#                                    `npm run build:docs:js`,
#                                    `npm run build:docs:python` - this
#                                    mode is just those three plus the
#                                    dist/docs/ assembly step.
#   bash .devops/build.sh python  - installs the native Python bindings
#                                    (`pip install .`) against the deps
#                                    `npm run prep:native` sets up under
#                                    build/native/ - what `npm run
#                                    build:python` runs. A real script,
#                                    not an inline command in package.json
#                                    like it used to be, specifically so
#                                    it always runs under bash (needed for
#                                    $(pwd) below to actually expand --
#                                    npm's default script shell on Windows
#                                    is cmd.exe, which has no such thing).
#   bash .devops/build.sh fuzz    - configures + builds the libFuzzer
#                                    harnesses (Clang required - see
#                                    tests/fuzz/README.md) into
#                                    build/native/fuzz/bin/, against the
#                                    same deps `npm run prep:native` sets
#                                    up (reused, not rebuilt with Clang --
#                                    see this mode's own comment on why
#                                    that's fine). What `npm run
#                                    build:native:fuzz` runs; same $(pwd)
#                                    reasoning as python above for why this
#                                    is a real script, not inlined into
#                                    package.json.
# Any arguments after the mode (and, for android, the ABI) are forwarded
# straight to the cmake configure step, e.g.:
#   bash .devops/build.sh native -DFlowerEvolver_BUILD_STATIC=FALSE -DFlowerEvolver_BUILD_TESTS=ON
#   bash .devops/build.sh android arm64-v8a -DFlowerEvolver_BUILD_STATIC=FALSE
# The mode itself defaults to "wasm" if omitted, so
# `bash .devops/build.sh -DSomeFlag=1` (no mode) still works as expected.
#
# Assumes it is run from the repository root (same assumption prep.sh
# and the other .devops/*.sh scripts already make).

set -e

MODE="wasm"
if [ "$1" = "wasm" ] || [ "$1" = "native" ] || [ "$1" = "android" ] || [ "$1" = "docs" ] || [ "$1" = "python" ] || [ "$1" = "fuzz" ]; then
    MODE="$1"
    shift
fi

if [ "$MODE" = "android" ]; then
    ANDROID_ABI="${1:-}"
    if [ -z "$ANDROID_ABI" ] || [[ "$ANDROID_ABI" == -* ]]; then
        echo "error: 'build.sh android' needs an ABI as its next argument, e.g.: bash .devops/build.sh android arm64-v8a" >&2
        exit 1
    fi
    shift
fi
EXTRA_CMAKE_ARGS=("$@")

# Default every configure step below to -G Ninja, unless the caller already
# passed their own -G in EXTRA_CMAKE_ARGS (e.g. -G "Unix Makefiles" to
# override it).
HAS_GENERATOR_ARG=false
for arg in "${EXTRA_CMAKE_ARGS[@]}"; do
    if [ "$arg" = "-G" ] || [[ "$arg" == -G* ]]; then
        HAS_GENERATOR_ARG=true
        break
    fi
done
GENERATOR_ARGS=()
if [ "$HAS_GENERATOR_ARG" = false ]; then
    GENERATOR_ARGS=(-G Ninja)
fi

case "$MODE" in
    wasm)
        if [ ! -f build/wasm/emsdk/emsdk_env.sh ]; then
            echo "error: emsdk not found under build/wasm/ -- run 'npm run prep' first." >&2
            exit 1
        fi
        source build/wasm/emsdk/emsdk_env.sh

        emcmake cmake -S . -B build/wasm \
            "${GENERATOR_ARGS[@]}" \
            -DEvoAI_ROOT="$(pwd)/build/wasm/EvoAI/build/install" \
            -DJsonBox_ROOT="$(pwd)/build/wasm/JsonBox/build/install" \
            "${EXTRA_CMAKE_ARGS[@]}"
        npm run build:ts
        cmake --build build/wasm --target dist -j 4
        echo "build.sh (wasm) done: dist/ ready."
        ;;
    native)
        if [ ! -d build/native/EvoAI/build/install ] || [ ! -d build/native/JsonBox/build/install ]; then
            echo "error: native EvoAI/JsonBox not found under build/native/ -- run 'npm run prep:native' first." >&2
            exit 1
        fi
        mkdir -p build/native/fe
        cmake -S . -B build/native/fe \
            "${GENERATOR_ARGS[@]}" \
            -DCMAKE_BUILD_TYPE=Release \
            -DEvoAI_ROOT="$(pwd)/build/native/EvoAI/build/install" \
            -DJsonBox_ROOT="$(pwd)/build/native/JsonBox/build/install" \
            "${EXTRA_CMAKE_ARGS[@]}"
        cmake --build build/native/fe -j 4

        echo "build.sh (native) done: build/native/fe/ ready."
        ;;
    android)
        if [ -z "$ANDROID_NDK" ]; then
            echo "error: \$ANDROID_NDK is not set -- point it at your Android NDK install." >&2
            exit 1
        fi
        ANDROID_API="${ANDROID_API:-33}"
        EvoAI_ROOT="$ANDROID_NDK/sources/third_party/EvoAI"
        JsonBox_ROOT="$ANDROID_NDK/sources/third_party/JsonBox"
        if [ ! -d "$EvoAI_ROOT/lib/$ANDROID_ABI" ] || [ ! -d "$JsonBox_ROOT/lib/$ANDROID_ABI" ]; then
            echo "error: EvoAI/JsonBox for $ANDROID_ABI not found -- run 'npm run prep:android -- $ANDROID_ABI' first (or 'bash .devops/prep.sh android $ANDROID_ABI')." >&2
            exit 1
        fi

        mkdir -p "build/android/fe-$ANDROID_ABI"
        cmake -S . -B "build/android/fe-$ANDROID_ABI" \
            "${GENERATOR_ARGS[@]}" \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_SYSTEM_NAME=Android \
            -DCMAKE_ANDROID_NDK="$ANDROID_NDK" \
            -DCMAKE_ANDROID_ARCH_ABI="$ANDROID_ABI" \
            -DCMAKE_ANDROID_API="$ANDROID_API" \
            -DCMAKE_ANDROID_STL_TYPE=c++_shared \
            -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=clang \
            -DEvoAI_ROOT="$EvoAI_ROOT" \
            -DJsonBox_ROOT="$JsonBox_ROOT" \
            "${EXTRA_CMAKE_ARGS[@]}"
        cmake --build "build/android/fe-$ANDROID_ABI" -j 4

        echo "build.sh (android/$ANDROID_ABI) done: build/android/fe-$ANDROID_ABI/ ready."
        ;;
    docs)
        if [ ! -f docs/CMakeLists.txt ]; then
            echo "error: docs/CMakeLists.txt not found -- run this from the repository root." >&2
            exit 1
        fi
        npm run build:docs:cpp
        npm run build:docs:js
        if python -c "import flowerevolver" >/dev/null 2>&1; then
            npm run build:docs:python
        else
            echo "warning: flowerevolver not importable -- skipping Python docs (run 'npm run prep:native && npm run build:python' first to include them)." >&2
        fi

        mkdir -p dist/docs
        rm -rf dist/docs/cpp dist/docs/js
        cp -r build/docs/html dist/docs/cpp
        cp -r build/docs-js dist/docs/js
        if [ -d build/docs-python ]; then
            rm -rf dist/docs/python
            cp -r build/docs-python dist/docs/python
            echo "build.sh (docs) done: dist/docs/{cpp,js,python}/ ready."
        else
            echo "build.sh (docs) done: dist/docs/{cpp,js}/ ready (python skipped -- see warning above)."
        fi
        ;;
    python)
        if [ ! -d build/native/EvoAI/build/install ] || [ ! -d build/native/JsonBox/build/install ]; then
            echo "error: native EvoAI/JsonBox not found under build/native/ -- run 'npm run prep:native' first." >&2
            exit 1
        fi
        pip install . -Cbuild-dir=build/python \
            -Ccmake.define.EvoAI_ROOT="$(pwd)/build/native/EvoAI/build/install" \
            -Ccmake.define.JsonBox_ROOT="$(pwd)/build/native/JsonBox/build/install" \
            "${EXTRA_CMAKE_ARGS[@]}"

        echo "build.sh (python) done: flowerevolver installed (import flowerevolver to use it, or run the flower-evolver console script)."
        ;;
    fuzz)
        if [ ! -d build/native/EvoAI/build/install ] || [ ! -d build/native/JsonBox/build/install ]; then
            echo "error: native EvoAI/JsonBox not found under build/native/ -- run 'npm run prep:native' first." >&2
            exit 1
        fi
        mkdir -p build/native/fuzz
        cmake -S . -B build/native/fuzz \
            "${GENERATOR_ARGS[@]}" \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DFlowerEvolver_BUILD_FUZZERS=ON \
            -DEvoAI_ROOT="$(pwd)/build/native/EvoAI/build/install" \
            -DJsonBox_ROOT="$(pwd)/build/native/JsonBox/build/install" \
            "${EXTRA_CMAKE_ARGS[@]}"
        cmake --build build/native/fuzz -j 4

        echo "build.sh (fuzz) done: build/native/fuzz/bin/fuzz_{flower_from_json,dna_from_json,stats_from_genome} ready -- see tests/fuzz/README.md to run them."
        ;;
    *)
        echo "build.sh: unknown mode '$MODE' (expected 'wasm', 'native', 'android', 'docs', 'python', or 'fuzz')" >&2
        exit 1
        ;;
esac
