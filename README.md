# Flower Evolver WASM

![cd](https://github.com/cristianglezm/FlowerEvolver-WASM/workflows/cd/badge.svg?branch=master)

Adapted Flower Evolver code from [EcoSystem](https://github.com/cristianglezm/EcoSystem.git) to use it in the browser without needing a backend.

check the website [here](https://cristianglezm.github.io/FlowerEvolver-WASM/index.html)

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
	git clone https://github.com/cristianglezm/EvoAI.git
	cd EvoAI && git checkout refactor-activations
	mkdir build && cd build
	emcmake -DEvoAI_BUILD_STATIC=TRUE -DJsonBox_ROOT=../JsonBox/build/install -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..
	make -j4 install
	cd ..
	cmake -DEvoAI_ROOT=EvoAI/build/install -DJsonBox_ROOT=JsonBox/build/install ..
	make -j4 dist
	cd ../dist && emrun index.html
```

# License

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
 