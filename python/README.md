# FlowerEvolver

Native Python bindings for [FlowerEvolver-WASM](https://github.com/cristianglezm/FlowerEvolver-WASM) - generates 2D/3D flowers from CPPN genomes evolved via [EvoAI](https://github.com/cristianglezm/EvoAI).

Genomes and rendered images round-trip with FlowerEvolver-WASM's own JS/WASM API and the native desktop app - `Flower.to_json()`/`Flower.from_json()` use the same `{"Flower": {...}}` wire format throughout.

## Install

Requires JsonBox and EvoAI already built, reuses the exact same build the native C++ library/tests use.

```bash
# builds JsonBox/EvoAI under build/native/, if not already done
bash .devops/prep.sh native

pip install . -Cbuild-dir=build/python \
    -Ccmake.define.EvoAI_ROOT=$(pwd)/build/native/EvoAI/build/install \
    -Ccmake.define.JsonBox_ROOT=$(pwd)/build/native/JsonBox/build/install
```

### Android (Termux)

Not an officially supported/tested platform, but reported working. Termux's Python doesn't expose the Python C-API symbols (`PyExc_ImportError` and friends) back to a `dlopen`'d extension the way a typical glibc Linux build does, so the nanobind extension fails to import with `dlopen failed: cannot locate symbol "PyExc_ImportError"` unless it's linked against `libpython` explicitly:

```bash
LDFLAGS="-lm -lpython3.14" npm run build:python
```

(adjust `python3.14` to whatever `python3 --version` reports on your install).

## Usage

```python
from pathlib import Path
from flowerevolver import make_flower, reproduce, mutate

# make two flowers, breed them, mutate the child
a = make_flower(radius=64, num_layers=3, P=6.0, bias=1.0)
b = make_flower(radius=64, num_layers=3, P=6.0, bias=1.0)
child = reproduce(a.dna, b.dna, radius=64, num_layers=3, P=6.0, bias=1.0)
mutant = mutate(child.dna, radius=64, num_layers=3, P=6.0, bias=1.0)

# save an image and the genome
Path("flower.png").write_bytes(mutant.petals.image.to_png_bytes())
Path("flower.json").write_text(mutant.to_json())

# ... and load it back later
from flowerevolver import Flower
loaded = Flower.from_json(Path("flower.json").read_text())
```

Every class and function has a docstring (`help(flowerevolver.make_flower)`, etc.), and the same content is published as a browsable API reference at the [Python docs site](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/) (Sphinx + autodoc, matching the C++/JS sites -- see `docs/python-conf/`). Build it locally with:

```bash
pip install -r docs/requirements.txt
npm run build:docs:python
```

(after the install steps above -- autodoc needs the real package importable, unlike the C++/JS sites, which only read source text; see `.devops/build.sh`'s `docs` mode for how this fits alongside those two, or `package.json`'s `build:docs:python` script if you'd rather skip npm and call `sphinx-build`/`python -m sphinx` directly.)

## CLI

Installing the package also installs a `flower-evolver` command, matching the native desktop app's own single-flower flags - minus `-cli`: the native app has that flag to pick its flag-driven single-flower path instead of launching its GUI, but this binding has no GUI to disambiguate from in the first place, so generating a flower is simply what happens by default:

```bash
# create a new flower, print its genome to stdout
flower-evolver -l 3 -r 64 -p 6.0 -b 1.0

# create one, save the genome + image to disk
flower-evolver -l 3 -r 64 -p 6.0 -b 1.0 -sf flower.json -si flower.png

# also generate + save a 3D model (glTF) alongside the 2D image
flower-evolver -l 3 -r 64 -p 6.0 -b 1.0 -sf flower.json -si flower.png -s3d flower.gltf

# ...or an emissive 3D model (useEmissive=true) instead, or both at once
flower-evolver -l 3 -r 64 -p 6.0 -b 1.0 -sf flower.json -si flower.png -s3d flower.gltf -se3d flower_emissive.gltf

# load a flower, mutate it 5 times, save only the final result - matches
# the native app: -m mutates in a loop and saves once at the end,
# Run this repeatedly (feeding each -sf output back in
# via -lf) if you want every intermediate step saved instead.
flower-evolver -lf flower.json -l 3 -r 64 -p 6.0 -b 1.0 -m 5 -sf flower.json -si flower.png

# reproduce two flowers, mutate the child twice, keep only the final result
flower-evolver -repr flower1.json flower2.json -l 3 -r 64 -p 6.0 -b 1.0 -m 2 -sf kidFlower.json -si kidFlower.png

flower-evolver --help
```

`-s3d <filename>`/`-se3d <filename>` are new to this Python CLI - the native app and JS demo both offer 3D export, but as its own separate action (a "Download 3D" button, distinct from "Download Image"), not a CLI flag; they mirror `-sf`/`-si`'s own "save this if you pass a path for it" shape rather than introducing a different kind of flag just for these two outputs. `-s3d` matches `FEService.make3DFlower()` (useEmissive=false); `-se3d` matches `FEService.makeEmissive3DFlower()` (useEmissive=true) -- pass either, or both, to save one or two models from the same flower. Both currently always render `sex="both"` with normals on (matching the JS demo's own default 3D view) -- narrower than `FEService`'s full `make3DFlower`/`makeEmissive3DFlower`/`draw3DFlower` trio in that per-flower sex control and reading params back out of the genome (what `draw3DFlower` does) aren't exposed here; widen it if that turns out to matter for the CLI too.

## Tests

```bash
pip install pytest
pytest python/tests/
```
