# glTF Flower Importer with Lights (FlowerEvolver)

A Blender add-on that imports emissive flowers exported by **FlowerEvolver-WASM** and automatically spawns lights based on each flower glTF extras.

## Requirements

- Blender 2.80 or later
- Python 3.7+ (bundled with Blender)
- A glTF file exported by a FlowerEvolver workflow (with `lights` metadata)

## Installation

1. copy the python file from this folder
2. paste the file into blender addon folder
3. enable it in the addons preferences.

## Usage

1. In Blender’s 3D View, navigate to **File > Import > glTF 2.0 Flower With lights (FlowerEvolver)**
2. Locate and select your `.gltf` file
3. Toggle **Import Extras** to import all glTF extras as custom properties
4. Click **Import glTF 2.0 (FlowerEvolver Lights)**
5. Watch as the Flower appears, each petal with its matching Blender light.
