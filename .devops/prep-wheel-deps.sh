#!/bin/bash
#
# Builds JsonBox + EvoAI (PIC static) for cibuildwheel's CIBW_BEFORE_ALL
# hook - see .github/workflows/ci.yml.
#
# Installs to $DEPS_ROOT/{JsonBox,EvoAI}/build/install, matching the
# layout used by .devops/prep.sh. Keep these paths in sync with the
# corresponding CIBW_CONFIG_SETTINGS/CMAKE_ARGS in ci.yml.
#
# $DEPS_ROOT resolves in this order: $1 (positional override) > the
# $DEPS_ROOT env var > /tmp/fe-native-deps.
#
# ci.yml deliberately does NOT rely on the plain /tmp default for any
# of its three wheel-building jobs anymore:
#   - Linux/macOS (cibuildwheel/Docker): passed explicitly as
#     {project}/build/native-deps, since a path under the project
#     checkout is guaranteed to be the same location cibuildwheel
#     mounts/copies into the container for both CIBW_BEFORE_ALL and the
#     actual build step.
#   - Windows MSVC / sdist (native, no Docker): passed via the
#     $DEPS_ROOT env var as ${{ github.workspace }}/build/native-deps.
# The /tmp default below only still matters for ad-hoc/local use.

set -e

if ! command -v git >/dev/null 2>&1; then
    if command -v apk >/dev/null 2>&1; then
        apk add --no-cache git
    else
        echo "error: git not found, and no apk to install it with - unexpected build environment (expected manylinux, musllinux, or macOS)." >&2
        exit 1
    fi
fi

DEPS_ROOT="${1:-${DEPS_ROOT:-/tmp/fe-native-deps}}"
mkdir -p "$DEPS_ROOT"
DEPS_ROOT="$(cd "$DEPS_ROOT" && pwd)"
cd "$DEPS_ROOT"

if [ ! -d JsonBox ]; then
    git clone https://github.com/cristianglezm/JsonBox.git
fi

mkdir -p JsonBox/build
cd JsonBox/build
cmake \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$(pwd)/install" \
    ..
cmake --build . -j 4 --config Release --target install

cd "$DEPS_ROOT"

if [ ! -d EvoAI ]; then
    git clone https://github.com/cristianglezm/EvoAI.git
fi

mkdir -p EvoAI/build
cd EvoAI/build
cmake \
    -DEvoAI_BUILD_STATIC=TRUE \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DJsonBox_ROOT="$DEPS_ROOT/JsonBox/build/install" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$(pwd)/install" \
    ..
cmake --build . -j 4 --config Release --target install

echo "prep-wheel-deps.sh done: JsonBox + EvoAI installed under $DEPS_ROOT/"
