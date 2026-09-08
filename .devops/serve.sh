#!/bin/bash
#
# Serves dist/ for local demo testing via emrun. emrun itself comes from
# emsdk (build/wasm/emsdk/), not from any system PATH, so it has to be
# sourced first.
# Needs `npm run prep` to have been run at least once (for the emsdk under
# build/wasm/) and `npm run build` to have produced dist/.

set -e

if [ ! -f build/wasm/emsdk/emsdk_env.sh ]; then
    echo "error: emsdk not found under build/wasm/ -- run 'npm run prep' first." >&2
    exit 1
fi
if [ ! -f dist/index.html ]; then
    echo "error: dist/index.html not found -- run 'npm run build' first." >&2
    exit 1
fi

source build/wasm/emsdk/emsdk_env.sh
emrun --serve_root dist index.html
