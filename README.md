# Flower Evolver WASM

[![cd](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/cd.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/cd.yml)
[![pkg](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg.yml)
[![NPM Version](https://img.shields.io/npm/v/%40cristianglezm%2Fflower-evolver-wasm?logo=npm)](https://www.npmjs.com/package/@cristianglezm/flower-evolver-wasm)
[![GitHub package.json version (branch)](https://img.shields.io/github/package-json/v/cristianglezm/flowerEvolver-wasm/master?logo=github)](https://github.com/cristianglezm/FlowerEvolver-WASM/pkgs/npm/flower-evolver-wasm)

Adapted Flower Evolver code from [EcoSystem](https://github.com/cristianglezm/EcoSystem.git) to be able to make, mutate, reproduce flowers in the browser without needing a backend.

check the website [here](https://cristianglezm.github.io/FlowerEvolver-WASM/)

> [!IMPORTANT]
> You will need to have a canvas object with id="canvas" to use the wasm module directly
> or if you're running the wasm module directly inside a web worker you will need 
> to initialize at the start: self.canvas = new OffscreenCanvas(radius * 2, radius * 3);
> if you use FEService, you don't need a canvas, it will auto inject a canvas object into self or document.body

## Building

Follow these steps to download, build, and run the project.

### Automated Build Process:

Use the `npm run build` command to automatically clone all dependencies and build the project.

Alternatively, follow the manual steps below:

### Step-by-step Manual Build:

```bash
# 1. Clone and set up Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest

# If using modules, install TypeScript Compiler
cd upstream/emscripten
npm install typescript
cd ../../

# Set up Emscripten environment
source ./emsdk_env.sh
cd ..

# 2. Clone and build JsonBox
git clone https://github.com/cristianglezm/JsonBox.git
cd JsonBox
mkdir build && cd build
emcmake cmake -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
make -j4 install
cd ..

# 3. Clone and build EvoAI
git clone https://github.com/cristianglezm/EvoAI.git
cd EvoAI
mkdir build && cd build
emcmake cmake -DEvoAI_BUILD_STATIC=TRUE -DJsonBox_ROOT=../JsonBox/build/install -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
make -j4 install
cd ..

# 4. Build the FlowerEvolver project
emcmake cmake -DFE_EXPORT=FALSE -DEvoAI_ROOT=EvoAI/build/install -DJsonBox_ROOT=JsonBox/build/install ..
make -j4 dist

# 5. Launch the demo website.
cd ../dist
emrun index.html
```

### Using npm run serve

After running ```npm run build``` run ``npm run serve`` to view the demo website.

## Browser usage

To use the FlowerEvolver module in the browser, refer to `include/FlowerEvolver.hpp` to explore all available functions.

### Example Code

Below is a sample JavaScript snippet to load and use the WebAssembly module in your project:

```javascript
var Module = {};
fetch('public/FlowerEvolver.wasm')
   .then(response => response.arrayBuffer())
   .then(buffer => {
       Module.wasmBinary = buffer;
       let script = document.createElement('script');
       script.src = 'public/FlowerEvolver.js';
       script.setAttribute("type", "text/javascript");
       document.body.appendChild(script);
    });

// Once the module is loaded
// Example: Create a flower genome
const genome = Module.makeFlower(64, 3, 6.0, 1.0);
// ...
```

### Notes

* Make sure the public/FlowerEvolver.wasm and public/FlowerEvolver.js files are accessible from your project directory.
* **Reference FlowerEvolver.hpp:** This header file contains detailed documentation on all functions and their parameters. Reviewing it is necessary to fully utilize the module's capabilities.

## Framework Usage

install it

```bash
npm install @cristianglezm/flower-evolver-wasm
```

```javascript
import { FEParams, FEService } from '@cristianglezm/flower-evolver-wasm';

let fes = new FEService();
await fes.init();
```

You can integrate the service into a state manager like Pinia for better state management. Here's an example setup:

```javascript
import { FEParams, FEService } from '@cristianglezm/flower-evolver-wasm';

export const useFlowersStore = defineStore('FlowersStore', {
    state: () => ({
        fe: null,
    }),
    getters: {},
    actions: {
        async loadFE() {
            this.fe = new FEService();
            await this.fe.init();
        },
    },
});

// Example Usage:
const store = useFlowersStore();
await store.loadFE();
//...
```

## Worker Usage

### App Code

In your main application file, set up the worker and handle communication:

```javascript
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
    if (!FE) {
        FE = new FEService();
        await FE.init();
    }

    // Use the methods from the FEService instance
    let flower;
    try {
        // Create a flower (Flower class contains genome and image)
        flower = await FE.makeFlower(params.radius, params.numLayers, params.P, params.bias);
    } catch (e) {
        // Handle errors
        console.error(e);
    }

    // You can access the internal FE.canvas if you want, it will have the new flower image after calling await FE.makeFlower()
    self.postMessage({
        genome: flower.genome,
        image: flower.image
    });
};
```

### Notes

* Use FE.makeFlower and other methods from FEService for creating flowers or reproduce them or mutate them.
* Always validate the params passed to the worker to avoid unexpected errors.
* The worker can utilize the internal FE.canvas for advanced operations.

## API Reference

The API reference is [here](https://github.com/cristianglezm/FlowerEvolver-WASM/blob/master/index.js). This file serves as a bridge between the WebAssembly module and JavaScript, providing a streamlined interface for interacting with the module.

## License

SFML parts in include/SFML and src/SFML are under its own [license](include/SFML/license.md).

the rest of the code is licensed under apache 2.0

```
   Copyright 2023-2025 Cristian Gonzalez cristian.glez.m@gmail.com

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
 
