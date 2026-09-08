# FlowerEvolver-WASM - JS/WASM API

- **FEService** module - `import { FEService, FEParams, Flower } from '@cristianglezm/flower-evolver-wasm'`. Start here; this is what most consumers want.
- **wasm-module** module - `import fe from '@cristianglezm/flower-evolver-wasm/wasm'`. The raw Emscripten module `FEService` wraps - only needed for advanced/low-level use.

For the native C++ API, see the [C++ docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/cpp/) instead. For native Python bindings, see the [Python docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/).
