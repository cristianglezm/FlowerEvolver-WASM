# FlowerEvolver-WASM - General Overview and Agent Context

This repository contains **FlowerEvolver-WASM**, a C++17 library that generates,
mutates, and reproduces 2D/3D flowers from CPPN genomes evolved via
[EvoAI](https://github.com/cristianglezm/EvoAI), shipped as a WebAssembly module
for the browser (with native, Android, and Python builds alongside it).

1. Ask, don't assume. If something is unclear, ask before writing a single line. Never make silent assumptions about intent, architecture, or requirements. When running unattended, pick the most reasonable interpretation, proceed, and record the assumption rather than blocking.
2. Implement the simplest solution for simple problems, better solutions for harder problems. Do not over-engineer or add flexibility that isn't needed yet.
3. Don't touch unrelated code but please do surface bad code or design smells you discover with me so we can address them as a separate issue.
4. Flag uncertainty explicitly. If you're unsure about something, see point 1 above. If it makes sense to do so, conduct a small, localised and low-risk experiment and bring the hypothesis and results to me to discuss. Confidence without certainty causes more damage than admitting a gap.
5. I'm always open to ideas on better ways to do things. Please don't hesitate to suggest a better way, or one that has long lasting impact over a tactical change.
6. When writing comments, write why, not what, if the code is complex it's ok to explain what it does too, but most code doesn't need a comment at all. Don't overshare context I already gave you by repeating it in the comment (e.g. "this is for project X to do Y and Z"). Example: given context that a function computes a checksum for a network protocol -- don't write "// computes the checksum for project X's protocol" (that just restates what I told you); do write "// XOR-fold, not CRC: protocol only needs error *detection*, and this runs on every packet" (that explains a non-obvious why). Default to no comment at all unless there's a real why worth capturing.

When generating code or answering questions about this repository, use the following overview as context:

## Core Features and Architecture
- **Flower generation**: `fe::DNA` wraps two `EvoAI::Genome` CPPNs (a stats genome + a petals genome), evolved/mutated/crossed via EvoAI's NEAT machinery. `fe::Petals`/`fe::Flower` turn a genome into a rendered `fe::Image` (2D) or, via `fe::gltf`, a glTF 2.0 mesh (3D): contour extraction from the 2D render -> contour simplification -> mesh generation -> material/texture assembly.
- **No canvas dependency**: rendering writes straight into an in-memory `fe::Image` (RGBA buffer), encoded to PNG (`encodeImageToPngInMemory`) and base64 (`encodeToBase64`) entirely in C++. Nothing in the library touches `document`/`OffscreenCanvas`.
- **Layered C++ API, one implementation shared by every binding**:
  `fe::detail::impl_*` (`include/fe/detail/FlowerEvolver.hpp`, `src/fe/detail/FlowerEvolver.cpp`) is the *only* place generation logic lives -- no JSON, no Emscripten, no Python. `src/bindings/{native,emscripten,python}/` are thin, typed wrappers around those `impl_*` calls, one per consumer:
  - `native.cpp` -- `fe::` types in, `fe::` types out (see `include/fe/FlowerEvolver.hpp` for the documented public native API).
  - `emscripten.cpp` -- adds JSON (JsonBox) parse/serialize + base64-PNG glue for the JS side.
  - `python/*.cpp` -- nanobind bindings, snake_case, docstrings on everything.
  Never duplicate generation logic in a binding layer; add a new `impl_*` if a new binding needs new behavior.
- **Shared wire format**: genomes serialize as `{"Flower": {"dna": {"genomes": [...]}, "petals": {...}}}` across JS/WASM, native C++, and Python -- `Flower.to_json()`/`from_json()` (or the JS/native equivalents) must stay interoperable across all three. A bare `DNA` (no `Flower` wrapper) serializes as `{"genomes": [...]}`.

## Directory Structure
- **`include/fe.hpp`**: umbrella header including the whole public API in one go.
- **`include/fe/`, `src/fe/`**: the `fe::` core library (DNA, Flower, Petals, Image, Stats, MathUtils, and the vendored-and-renamed `Color`/`Rect`/`Vector2` -- these were originally SFML's `sf::` types, merged into `fe::` and moved here specifically so this library never collides with a consumer's own real SFML; see `include/fe/SFML-LICENSE.md` for the required attribution that move still carries). Nested under `fe/`.
- **`include/fe/detail/`, `src/fe/detail/`**: the `fe::detail::impl_*` shared generation logic (see above).
- **`include/fe/3D/`, `src/fe/3D/`**: contour/mesh/glTF generation for 3D flowers (`fe::gltf` namespace).
- **`src/bindings/{emscripten,native,python}/`**: per-consumer thin wrapper layers.
- **`index.ts`**: the `FEService` TypeScript wrapper -- the JS-facing API most web consumers want; ships its own generated `index.d.ts`.
- **`python/flowerevolver/`**: the Python package (`__init__.py` re-exports from the nanobind `_core` extension) plus `cli.py` (the `flower-evolver` console script). Import name (`flowerevolver`) and PyPI distribution name are deliberately identical - `cli.py`/`__init__.py` resolve the installed version via `importlib.metadata.version(__name__)`, which only works when the distribution name matches `__name__` exactly. The console script name (`flower-evolver`) is unrelated to either and can differ.
- **`tests/{cpp,js,e2e,fuzz}/`, `python/tests/`**: Catch2, vitest, Playwright, libFuzzer, and pytest suites respectively.
- **`docs/`**: Doxygen+Breathe+Sphinx (C++), TypeDoc (JS/WASM), Sphinx+autodoc (Python) -- three sites, cross-linked, deployed under `dist/docs/{cpp,js,python}/`.
- **`.devops/`**: `prep.sh` (one-time dependency setup: emsdk/JsonBox/EvoAI, per target -- `wasm`/`native`/`android`) and `build.sh` (repeatable configure+build, per target -- `wasm`/`native`/`android`/`docs`/`python`/`fuzz`). `npm run prep`/`npm run build` are just these two scripts; see their own top-of-file comments for exactly what each mode does before reaching for a manual `cmake` invocation.
- **`cmake/modules/`**: `FindJsonBox.cmake`/`FindEvoAI.cmake` (neither dependency ships its own discoverable CMake package the default way we'd want, hence hand-written Find modules) and `FindSphinx.cmake`. `cmake/FlowerEvolverConfig.cmake.in` is this project's *own* exported package config (`find_package(FlowerEvolver)` -> `FlowerEvolver::FlowerEvolver`), not a Find module for a dependency.

## Development Guidelines
- **C++ Standard**: C++17.
- **Dependencies**: JsonBox (serialization) and EvoAI (NEAT/CPPN genomes, itself depending on JsonBox) -- both built from source via `.devops/prep.sh`, never assumed to be system-installed. EvoAI is always linked static (`EvoAI_BUILD_STATIC=TRUE`) regardless of whether FlowerEvolver itself builds shared or static.
- **Build System**: CMake for the wasm/native/Android targets (one target per configure -- see `FlowerEvolver_BUILD_STATIC`, never both shared and static from the same configure). npm scripts are thin wrappers around `.devops/*.sh` for the JS/WASM/demo/docs side. Python packaging goes through `scikit-build-core` + `nanobind` (`pyproject.toml`), driving `python/CMakeLists.txt`.
- **`FE_API` (in `config.hpp`, generated from `config.hpp.template`)**: required on every class/free function in the public headers that needs to be visible across a shared-library boundary -- expands to `__declspec(dllexport)`/`dllimport` on Windows shared builds, `__attribute__((visibility("default")))` on GCC/Clang shared builds, and nothing at all when `FE_STATIC` is defined (Emscripten and static native builds both define it). Forgetting `FE_API` on a new public symbol only breaks the Windows *shared* build -- easy to miss on Linux/macOS.
- **The "needs >= 2 genomes" invariant**: a lot of entry points (`Flower`'s JSON constructor, `drawFlower`/`drawPetals`/`drawPetalLayer`, `getFlowerStats`, `make3DFlower`, `reproduce`, `mutate`) take a `DNA` and immediately need `dna.size() >= 2` (stats genome + petals genome) to do anything meaningful, and throw `std::invalid_argument` otherwise. `DNA::operator[]` itself is unchecked by design (matches the exception-vs-crash trade-off already made elsewhere in the library) -- so when adding a new DNA-consuming entry point, add this same guard rather than relying on the caller to have validated it, and add a fuzz/unit-test case for the under-2-genomes input specifically.
- **`mutate()` never changes genome count**: `DNA::mutate()` (and the free `mutate()`/`impl_mutate()` wrapping it) perturbs each *existing* genome's topology/weights in place -- it never adds or removes genomes. A test asserting "mutate leaves the input untouched" that only checks `size()` proves nothing (size never changes either way); compare a full JSON/state snapshot instead if that's actually what's under test.
- **Docstrings/doxygen**: `include/fe/FlowerEvolver.hpp` (native) and `types/wasm-module.d.ts` (JS/WASM) are the two hand-maintained sources of truth for their respective public APIs -- `include/fe/detail/FlowerEvolver.hpp` and `src/bindings/native.cpp` deliberately don't re-document the same parameters a second time (nothing to keep in sync if it's only written once). Python bindings (`src/bindings/python/*.cpp`) get their own full docstrings via nanobind, since `help()`/Sphinx autodoc read those directly rather than the C++ header comments.
