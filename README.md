# Flower Evolver WASM

[![cd](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/cd.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/cd.yml)
[![pkg](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg.yml/badge.svg?branch=master)](https://github.com/cristianglezm/FlowerEvolver-WASM/actions/workflows/pkg.yml)

Adapted Flower Evolver code from [EcoSystem](https://github.com/cristianglezm/EcoSystem.git) to use it in the browser without needing a backend.

check the website [here](https://cristianglezm.github.io/FlowerEvolver-WASM/)

> [!IMPORTANT]
> You will need to have a canvas object with id="canvas" to use the wasm functions.

## Building

You will need [emsdk](https://github.com/emscripten-core/emsdk) to build [JsonBox](https://github.com/cristianglezm/JsonBox) and [EvoAI](https://github.com/cristianglezm/EvoAI)

```bash
	git clone https://github.com/emscripten-core/emsdk.git
	cd emsdk && ./emsdk install latest && ./emsdk activate latest
	source ./emsdk_env.sh
	cd ..
	git clone https://github.com/cristianglezm/JsonBox.git
	cd JsonBox && mkdir build && cd build
	emcmake cmake -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
	make -j4 install
	cd ..
	# you will need permision for EvoAI (it will soon be open sourced)
	git clone https://github.com/cristianglezm/EvoAI.git
	cd EvoAI && git checkout refactor-activations
	mkdir build && cd build
	emcmake cmake -DEvoAI_BUILD_STATIC=TRUE -DJsonBox_ROOT=../JsonBox/build/install -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
	make -j4 install
	cd ..
	emcmake cmake -DFE_EXPORT=FALSE -DEvoAI_ROOT=EvoAI/build/install -DJsonBox_ROOT=JsonBox/build/install ..
	make -j4 dist
	cd ../dist && emrun index.html
```

## Browser usage

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
// then after is loaded
// const genome = Module.makeFlower(64, 3, 6.0, 1.0);
// ...
````

## Framework usage

install it

npm install @cristianglezm/flower-evolver-wasm

```javascript
import fe from '@cristianglezm/flower-evolver-wasm';

//in a state manager like pinia:

export const useFlowersStore = defineStore('FlowersStore', {
	state: () => ({
		fe: null,
		canvas: document.getElementById("canvas"),
        }),
        getters: {},
        actions:{
            async loadFE(){
                this.fe = await fe();
            },
        });
//then when using the store you will need to call
const store = useFlowersStore(pinia);
store.loadFE();
//...
````

## License

SFML parts in include/SFML and src/SFML are under its own [license](include/SFML/license.md).

the rest of the code is licensed under apache 2.0

```
   Copyright 2023 Cristian Gonzalez cristian.glez.m@gmail.com

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
 
