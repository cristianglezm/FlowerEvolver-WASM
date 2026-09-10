# FlowerEvolver-WASM

[![ci](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/ci.yml)
[![cd](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/cd.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/cd.yml)
[![pkg](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg.yml)
[![pkg-python](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg-python.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg-python.yml)
[![NPM Version](https://img.shields.io/npm/v/%40cristianglezm%2Fflower-evolver-wasm?logo=npm)](https://www.npmjs.com/package/@cristianglezm/flower-evolver-wasm)
[![PyPI Version](https://img.shields.io/pypi/v/flowerevolver?logo=pypi)](https://pypi.org/project/flowerevolver/)
[![GitHub package.json version (branch)](https://img.shields.io/github/package-json/v/cristianglezm/flowerEvolver-wasm/master?logo=github)](https://github.com/cristianglezm/FlowerEvolver-WASM/pkgs/npm/flower-evolver-wasm)

Generates, mutates, and reproduces 2D/3D flowers from CPPN genomes evolved via [EvoAI](https://github.com/cristianglezm/EvoAI) - adapted from [EcoSystem](https://github.com/cristianglezm/EcoSystem.git). The core is a single C++17 library (`fe::`) shipped as a WebAssembly module for the browser, a native static/shared library for C++ apps, native Python bindings, and an Android build - one implementation, four ways to consume it.

The CPPN-encoded, evolvable flower approach the project implements follows the one introduced by Risi et al. in *Petalz* - see [Acknowledgments](#acknowledgments).

Check the live demo [here](https://cristianglezm.github.io/FlowerEvolver-WASM/).

## Contents

* [News](#news)
* [Building](#building)
* [Browser Usage](#browser-usage)
* [Framework Usage](#framework-usage)
* [Worker Usage](#worker-usage)
* [Android](#android)
* [Native C++ Library](#native-c-library)
* [Python Bindings](#python-bindings)
* [Documentation](#documentation)
* [Testing](#testing)
* [API Reference](#api-reference)
* [Acknowledgments](#acknowledgments)
* [License](#license)

## News

<details>
<summary><strong>v4.0.0</strong> - native CMake build, Python bindings, TypeScript rewrite, docs site, and more (click to expand)</summary>

**Additions**
* Native CMake build (Linux/macOS/Windows) alongside the Emscripten/WASM target, so `fe::` can be linked directly into a C++ app -- see "Building" below.
* Native Python bindings (nanobind) plus a `flower-evolver` CLI - see [python/README.md](python/README.md).
* Android build via CMake.
* `index.ts`: the JS-facing `FEService` wrapper is now TypeScript, shipping its own generated `index.d.ts`.
* Full API docs site (Doxygen+Sphinx for C++, TypeDoc for JS/WASM, Sphinx+autodoc for Python) - linked from the demo page.
* Test coverage: C++ (Catch2), JS/Node (vitest), e2e (Playwright), Python (pytest), and libFuzzer fuzzing harnesses.
* CI/CD: a native x wasm x Android build matrix, plus tag-triggered release automation for npm, GitHub Packages, and PyPI.

**Breaking changes**
* WASM no longer touches `document`/`OffscreenCanvas` at all -- `{genome, image}` (image as a base64 PNG data URL) comes back directly from the module. If you were reading pixels off an injected `<canvas id="canvas">` or `self.canvas`, switch to `FEService` (or read `.image` off its JS/WASM module's own JSON result) instead.
* The `FE_EXPORT` TRUE/FALSE build split is gone -- there's one Emscripten build now (ES6 module + `SINGLE_FILE`). `dist/public/FlowerEvolver.mjs` is now `dist/public/FlowerEvolver.js`, and `dist/index.js` is compiled from `index.ts` rather than hand-written.
* The native C++ API (`include/fe/FlowerEvolver.hpp`) now takes/returns typed `fe::DNA`/`fe::Flower`/`fe::Image` instead of stringified JSON -- only affects native (non-WASM) consumers; the JS/WASM API's own shape is unchanged.

</details>

<details>
<summary><strong>v3.0.0</strong> - 3D flower generation and stats (click to expand)</summary>

* 3D flower generation pipeline: 2D petal layers go through contour extraction, contour simplification, and mesh generation, and come out as a glTF 2.0 scene (mesh, material, texture, node) with petal droop, a generated pistil and stamens, and lighting extras.
* Procedural normal maps (from a noise function) and emissive textures for glowing flowers, plus `adjustParams`/`adjustStem` so stem/pistil proportions and per-layer petal droop scale adaptively with the flower's own size instead of using fixed constants.
* Layered radius scaling (`applyLayeredRadiusScaling` in `drawLayer()`) reuses the same petal image at progressively lower resolutions in 2D mode and pre-scales textures in 3D mode -- measured to cut VRAM use from 22MB to 2.2MB and file size from 1MB to 0.7MB at radius 256 / 20 layers.
* In-memory PNG encoding and base64 encoding, so textures can be embedded directly in the glTF file instead of round-tripping through the DOM.
* `Image` split into its own header/source file; `numLayers` is now clamped based on `radius` to avoid excessive memory allocation.
* Flower `Stats` generation (health, stamina, temperature range, maturation, toxicity, and per-stat effects) derived from a flower's genome.
* A companion Blender plugin, and 3D examples added to the demo page.

</details>

<details>
<summary><strong>v2.0.0 / v2.0.1</strong> - FEService wrapper (click to expand)</summary>

* Added `FEService`, `FEParams`, and `Flower` to `index.js` -- a documented JS wrapper around the raw embind calls (`makeFlower`/`reproduce`/`mutate`/etc.), instead of consumers calling the wasm module's exported functions directly and reading the canvas by hand.
* Added `getDataUrl`, a small helper to read the (real or offscreen) canvas back as a data URL, used internally by `FEService` and safe to call from a Web Worker.
* v2.0.1: explicitly exported `HEAPU8` from the Emscripten build, a same-day bugfix so `FEService` could reliably read pixel data back out of the wasm heap.

</details>

<details>
<summary><strong>v1.1.0</strong> - partial rendering (click to expand)</summary>

* Added `makePetals`/`makePetalLayer`/`makeStem` and their `draw*` counterparts, so a consumer can generate or redraw just the petals, a single petal layer, or just the stem, instead of always paying for a full flower render.

</details>

<details>
<summary><strong>v1.0.0 / v1.0.1</strong> - initial Emscripten module (click to expand)</summary>

* Initial release: the flower generator (genome, petals, drawing) compiled to WebAssembly via Emscripten/embind, driven against a `<canvas id="canvas">` element in the page.
* v1.0.1 added npm packaging (`package.json`, `.devops/dist.sh`, `pkg.yml`) with an `FE_EXPORT` flag to build either a plain global script or an ES6 module, and auto-publish to npm and GitHub Packages on release.

</details>

## Building

Follow these steps to download, build, and run the project.

### Automated Build Process

Run `npm run prep` once to set up the dependencies (Emscripten SDK, JsonBox, EvoAI, all under `build/wasm/`), then `npm run build` to configure and build the project. `npm run build` is safe to re-run as often as you like -- it doesn't redo the dependency setup, so it's fast for iterating. It also compiles `index.ts` (via `npm run build:ts`, run automatically as part of `npm run build`) -- run `npm run build:ts` on its own if you're only iterating on the JS-facing `FEService` wrapper and don't need a full wasm rebuild.

That's genuinely all most people need for the browser/npm package -- the table below covers the other targets (native, Android, Python, docs, fuzzing), and the rest of this section is only for readers who want to see what those two commands do underneath.

### Other build targets

Every target follows the same two-step shape: a one-time `prep` (clones and builds JsonBox/EvoAI for that target) and a repeatable `build`.

| Target | Setup (once) | Build | Notes |
|---|---|---|---|
| WASM (browser/npm) | `npm run prep` | `npm run build` | Default target; produces `dist/` |
| Native C++ library | `npm run prep:native` | `npm run build:native` | Add `-DFlowerEvolver_BUILD_TESTS=ON` (or use `npm run build:native:test`) to also build the Catch2 suite |
| Android (all 4 ABIs) | `npm run prep:android` | `npm run build:android -- <abi>` | One ABI per `build` invocation; needs `$ANDROID_NDK` -- see [Android](#android) |
| Python bindings | `npm run prep:native` | `npm run build:python` | nanobind; installs the `flowerevolver` package + `flower-evolver` CLI -- see [Python Bindings](#python-bindings) |
| Docs (C++ + JS + Python) | `npm run prep:native && npm run build:python` + `pip install -r docs/requirements.txt` | `npm run build:docs` | Doxygen+Sphinx (C++), TypeDoc (JS/WASM), Sphinx+autodoc (Python) -- Python docs need `build:python` first, see [Documentation](#documentation) |
| Fuzzing | `npm run prep:native` | `npm run build:native:fuzz` | Needs Clang -- see `tests/fuzz/README.md` |

`npm run prep` and `npm run build` are themselves just `bash .devops/prep.sh` and `bash .devops/build.sh` -- there's no separate manual recipe to keep in sync with those scripts, so if you want to see (or run) each underlying step individually, read `.devops/prep.sh` and `.devops/build.sh` directly. Both document every mode (`wasm`/`native`/`android`/`docs`/`python`/`fuzz`) in a comment block at the top, and both are safe to re-run as often as you like. For a manual, non-npm `cmake`/`cmake --build` invocation, see `cmake/modules/{FindEvoAI,FindJsonBox}.cmake` for the `_ROOT` variables it expects.

### Using npm run serve

After running `npm run prep` (once) and `npm run build`, run `npm run serve` to view the demo website.

## Browser usage

The build is a single self-contained ES module (the wasm binary is inlined as base64, no separate `.wasm` file to fetch) -- use it directly with an import map, no bundler required. For the full function reference (native C++ API), see `include/fe/FlowerEvolver.hpp`.

### Example Code

Below is a sample JavaScript snippet to load and use the module in your project:

```html
<script type="importmap">
{
  "imports": {
    "@cristianglezm/flower-evolver-wasm": "./node_modules/@cristianglezm/flower-evolver-wasm/index.js"
  }
}
</script>
<script type="module">
    import { FEService } from '@cristianglezm/flower-evolver-wasm';

    const fes = new FEService();
    await fes.init();

    // Example: create a flower
    const flower = await fes.makeFlower();
    // flower.genome - stringified genome, feed back into reproduce/mutate/draw*
    // flower.image  - base64-encoded PNG data URL, ready for an <img src="...">
</script>
```

### Notes

* **Reference FlowerEvolver.hpp:** This header file contains detailed documentation on all functions and their parameters (native C++ API). `index.ts`'s `FEService` is the recommended JS-facing wrapper -- see [API Reference](#api-reference) below.
* Written in TypeScript (`index.ts`), the published package ships its own `index.d.ts` (generated straight from that source) alongside `index.js`, so TypeScript consumers get full type checking and autocomplete with no extra `@types` package to install.

## Framework Usage

install it

```bash
npm install @cristianglezm/flower-evolver-wasm
```

```javascript
import { FEService } from '@cristianglezm/flower-evolver-wasm';

let fes = new FEService();
await fes.init();
```

You can integrate the service into a state manager like Pinia for better state management. Here's an example setup:

```javascript
import { defineStore } from 'pinia';
import { FEParams, FEService } from '@cristianglezm/flower-evolver-wasm';

export const useFlowersStore = defineStore('FlowersStore', {
    state: () => ({
        fe: null,
        flowers: [],
    }),
    getters: {},
    actions: {
        async loadFE() {
            this.fe = new FEService();
            await this.fe.init();
        },
        setParams(params) {
            // only called when you actually want to change something --
            // makeFlower() below reuses whatever was set last (or the
            // FEService default, if this was never called).
            this.fe.setParams(params);
        },
        async makeFlower() {
            const flower = await this.fe.makeFlower();
            this.flowers.push(flower);
            return flower;
        },
    },
});

// Example Usage:
const store = useFlowersStore();
await store.loadFE();
store.setParams(new FEParams(128, 2, 5.0, -1.0));
await store.makeFlower();
//...
```

## Worker Usage

### App Code

In your main application file, set up the worker and handle communication:

```javascript
import { FEParams } from '@cristianglezm/flower-evolver-wasm';

// Create a new Worker instance
let worker = new Worker('worker.js');

// Handle messages from the worker
worker.onmessage = (e) => {
    // Perform actions with the received data
};

// Send data to the worker
const fn = () => {
    worker.postMessage({ params: new FEParams(radius, numLayers, P, bias) });
};
```

### Worker code

In worker.js, initialize the FlowerEvolver service and process incoming messages:

```javascript
import { FEParams, FEService } from '@cristianglezm/flower-evolver-wasm';

let FE;

self.onmessage = async (e) => {
    /// validate params to avoid OOM errors.
    let params = e.data.params;
    // Initialize FlowerEvolver service if not already initialized
    if(!FE){
        FE = new FEService();
        await FE.init();
    }
    FE.setParams(new FEParams(params.radius, params.numLayers, 
                                params.P, params.bias));
    // Use the methods from the FEService instance
    let flower;
    try{
        // Create a flower (Flower class contains genome and image)
        flower = await FE.makeFlower();
    }catch(e){
        // Handle errors
        console.error(e);
    }

    // flower.genome and flower.image (base64 PNG data URL) are both ready
    // to use directly -- no canvas readback needed.
    self.postMessage({
        genome: flower.genome,
        image: flower.image
    });
};
```

### Notes

* Use FE.makeFlower and other methods from FEService for creating flowers or reproduce them or mutate them.
* Always validate the params passed to the worker to avoid unexpected errors.

## Android

The native library also cross-compiles for Android via CMake's own `CMAKE_SYSTEM_NAME=Android` support (no Android Studio/Gradle project needed).

```bash
# needs $ANDROID_NDK pointing at a real NDK install
npm run prep:android                 # all 4 Play-Store-relevant ABIs by default
npm run prep:android -- arm64-v8a    # or just the ABI(s) you need

npm run build:android -- arm64-v8a -DFlowerEvolver_BUILD_STATIC=FALSE
```

This produces `build/android/fe-<abi>/`, packaged with `cpack -G ZIP` in CI (see the `android`/`android-merge` jobs in `.github/workflows/ci.yml`), and a merged `FlowerEvolver-Android.zip` with all four ABIs side by side for release tags. `src/Android.mk` is included for classic `ndk-build` consumers who'd rather drop the prebuilt `.so`/`.a` straight into an existing NDK project than adopt CMake.

## Native C++ Library

Link `libFlowerEvolver` directly into a C++ app once it's built and installed (`npm run prep:native && npm run build:native`, then `cmake --install`):

```cmake
find_package(FlowerEvolver 4 REQUIRED)
target_link_libraries(myApp PRIVATE FlowerEvolver::FlowerEvolver)
```

```cpp
#include <fe/FlowerEvolver.hpp>

int main(){
    fe::Flower flower = makeFlower(64, 3, 6.0f, 1.0f);
    auto png = fe::encodeImageToPngInMemory(flower.petals.image);
    // flower.dna.toJson() for the genome; reproduce()/mutate() take fe::DNA in, fe::Flower out.
}
```

See `include/fe/FlowerEvolver.hpp` for the full documented API (same generation logic the WASM, Android, and Python bindings all call through `fe::detail::impl_*`), and `cmake/modules/{FindEvoAI,FindJsonBox}.cmake` for the dependency `_ROOT` variables a manual configure needs.

## Python Bindings

Native Python bindings (via [nanobind](https://github.com/wjakob/nanobind)) around the same C++ core -- same `Flower.to_json()`/`from_json()` wire format as the JS/WASM API above, so genomes round-trip between both. Also installs a `flower-evolver` CLI matching the native desktop app's own single-flower flags.

```bash
npm run prep:native   # once
npm run build:python
```

```python
from flowerevolver import make_flower

flower = make_flower(radius=64, num_layers=3, P=6.0, bias=1.0)
flower.petals.image.to_png_bytes()  # -> bytes
flower.to_json()                    # -> str, the genome
```

`npm run test:python` runs its pytest suite. See [python/README.md](python/README.md) for the manual `pip install` invocation, the CLI, and more usage examples; the C++ glue itself lives in `src/bindings/python/`.

## Documentation

Three cross-linked API reference sites, built from the same source the demo page itself links to:

* [C++ docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/cpp/) - Doxygen + Breathe + Sphinx, from `include/fe/FlowerEvolver.hpp` and the rest of the public `fe::` headers.
* [JS/WASM docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/js/) - TypeDoc, from `index.ts` and `types/wasm-module.d.ts`.
* [Python docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/) - Sphinx + autodoc, from the installed `flowerevolver` package's own docstrings.

Build them locally with `npm run build:docs` (needs `npm run prep:native` first, plus Doxygen/Graphviz and `pip install -r docs/requirements.txt`); output lands in `dist/docs/{cpp,js,python}/`. The Python site additionally needs the `flowerevolver` package itself importable -- run `npm run build:python` before `npm run build:docs`, or the C++ and JS sites build normally while the Python site is silently skipped (with a warning) instead of failing the whole command. Each site can also be built on its own -- `npm run build:docs:cpp`, `npm run build:docs:js`, `npm run build:docs:python` (the last one assumes `build:python` has already run).

## Testing

| Suite | Setup | Run |
|---|---|---|
| C++ (Catch2) | `npm run prep:native && npm run build:native:test` | `npm run test:cpp` |
| JS/Node (vitest, against a real build) | `npm run prep && npm run build` | `npm test` |
| e2e (Playwright, drives the built demo) | `npm run prep && npm run build` | `npm run test:e2e` |
| Python (pytest) | `npm run prep:native && npm run build:python` | `npm run test:python` |
| Fuzzing (libFuzzer, Clang) | `npm run prep:native && npm run build:native:fuzz` | see `tests/fuzz/README.md` |

## API Reference

* JS/WASM: [`index.ts`](https://github.com/cristianglezm/FlowerEvolver-WASM/blob/master/index.ts), or the browsable [JS/WASM docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/js/).
* Native C++: [`include/fe/FlowerEvolver.hpp`](https://github.com/cristianglezm/FlowerEvolver-WASM/blob/master/include/fe/FlowerEvolver.hpp), or the browsable [C++ docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/cpp/).
* Python: the browsable [Python docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/), or [python/README.md](python/README.md) for the CLI.

## Acknowledgments

The CPPN-encoded, breedable-flower approach implemented here follows the one introduced by:

> Risi, S., Lehman, J., D'Ambrosio, D., Hall, R., & Stanley, K. (2012). Combining Search-Based Procedural Content Generation and Social Gaming in the Petalz Video Game. *Proceedings of the AAAI Conference on Artificial Intelligence and Interactive Digital Entertainment*, 8(1). [https://doi.org/10.1609/aiide.v8i1.12502](https://doi.org/10.1609/aiide.v8i1.12502)

Genome evolution (NEAT/CPPN) itself is provided by [EvoAI](https://github.com/cristianglezm/EvoAI).

## License

SFML-derived parts (`fe::Color`/`fe::Rect`/`fe::Vector2`, adapted from SFML and merged into the `fe::` namespace) are under SFML's own [license](include/fe/SFML-LICENSE.md).

stb_image_write is under its own [license](include/stb_image_write.h)

the rest of the code is licensed under apache 2.0

```
   Copyright 2023-2026 Cristian Gonzalez <cristian.glez.m@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```
