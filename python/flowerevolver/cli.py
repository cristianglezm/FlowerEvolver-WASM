"""
FlowerEvolver CLI - matches the native desktop app's own single-flower
flags: -lf, -sf, -si, -s3d, -m, -repr, and the shared -l/-r/-p/-b
generation parameters.

Three entry points (each self-sufficient - -lf/-repr just replace "start
from a fresh flower" with "start from an existing one/two"). -m <n>
mutates n times and saves only the final result, matching the native app
- run this repeatedly (feeding each -sf output back in via -lf) if you
want every intermediate step saved instead:

    flower-evolver -l <numLayers> -r <radius> -p <P> -b <bias> -m <n>
    flower-evolver -lf <flower.json> -l <numLayers> -r <radius> -p <P> -b <bias> -m <n> -sf <out.json> -si <out.png>
    flower-evolver -repr <flower1.json> <flower2.json> -l <numLayers> -r <radius> -p <P> -b <bias> -m <n> -sf <out.json> -si <out.png>

Add -s3d <filename> to any of the above to also generate and save a 3D
model (glTF) for the resulting flower, e.g.:

    flower-evolver -sf flower.json -si flower.png -s3d flower.gltf

Use -se3d <filename> instead of (or alongside) -s3d for an emissive 3D
model - same glTF export, but with useEmissive turned on, matching
FEService.makeEmissive3DFlower()/drawEmissive3DFlower() on the JS/WASM
side. Both can be passed together to save a regular and an emissive
model from the same flower:

    flower-evolver -sf flower.json -si flower.png -s3d flower.gltf -se3d flower_emissive.gltf
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from . import Flower, get_version_string, make_3d_flower, make_flower, mutate as _mutate, reproduce as _reproduce


def _save(flower: Flower, save_flower: str | None, save_image: str | None) -> None:
    if save_flower:
        Path(save_flower).write_text(flower.to_json())
    if save_image:
        Path(save_image).write_bytes(flower.petals.image.to_png_bytes())
    if not save_flower and not save_image:
        print(flower.to_json())


def _save_3d(dna, radius: int, num_layers: int, P: float, bias: float, save_3d: str, *, emissive: bool = False) -> None:
    """
    Generates and saves a 3D model (glTF) for `dna` to `save_3d`.

    flower_id (required by make_3d_flower -- see its own docstring, used
    in the model's own group names) is derived from save_3d's filename
    stem: simplest thing that's still meaningful.

    `emissive` toggles useEmissive in the generated model's flowerParams
    -- matches FEService.makeEmissive3DFlower() (emissive=True) vs.
    make3DFlower() (emissive=False) on the JS/WASM side; -se3d passes
    True here, -s3d passes False.
    """
    flower_id = Path(save_3d).stem
    flower_params = json.dumps(
        {"sex": 2, "useNormals": True, "useEmissive": emissive})
    gltf = make_3d_flower(dna, radius, num_layers, P,
                          bias, flower_id, flower_params)
    Path(save_3d).write_text(gltf)


def _run(flower: Flower, mutations: int, radius: int, num_layers: int, P: float, bias: float,
         save_flower: str | None, save_image: str | None, save_3d: str | None,
         save_emissive_3d: str | None) -> None:
    for _ in range(mutations):
        flower = _mutate(flower.dna, radius, num_layers, P, bias)

    _save(flower, save_flower, save_image)
    if save_3d:
        _save_3d(flower.dna, radius, num_layers,
                 P, bias, save_3d, emissive=False)
    if save_emissive_3d:
        _save_3d(flower.dna, radius, num_layers, P,
                 bias, save_emissive_3d, emissive=True)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="flower-evolver",
        description="Generates 2D (and optionally 3D) flowers from CPPN genomes evolved via EvoAI.",
    )
    parser.add_argument("--version", action="version",
                        version=f"flower-evolver {get_version_string()}",
                        help="show the extension's version + build commit and exit")
    parser.add_argument("-lf", metavar="<filename>",
                        help="load an existing Flower instead of generating a new one")
    parser.add_argument("-repr", nargs=2, metavar=("<filename1>", "<filename2>"),
                        help="reproduces two flowers instead of generating a new one")
    parser.add_argument("-sf", metavar="<filename>",
                        help="saves json from Flower")
    parser.add_argument("-si", metavar="<filename>",
                        help="saves image from Flower")
    parser.add_argument("-s3d", metavar="<filename>",
                        help="also generates and saves a 3D model (glTF) for the flower")
    parser.add_argument("-se3d", metavar="<filename>",
                        help="also generates and saves an emissive 3D model (glTF, useEmissive=true) for the flower")
    parser.add_argument("-m", type=int, default=0, metavar="<n>",
                        help="mutates the flower n times")
    parser.add_argument("-l", type=int, default=3, metavar="<numLayers>",
                        help="how many layers the flowers have.")
    parser.add_argument("-r", type=int, default=64,
                        metavar="<radius>", help="radius of the flowers.")
    parser.add_argument("-p", type=float, default=6.0,
                        metavar="<P>", help="P parameter.")
    parser.add_argument("-b", type=float, default=1.0,
                        metavar="<bias>", help="bias parameter.")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        if args.repr:
            f1 = Flower.from_json(Path(args.repr[0]).read_text())
            f2 = Flower.from_json(Path(args.repr[1]).read_text())
            flower = _reproduce(f1.dna, f2.dna, args.r, args.l, args.p, args.b)
        elif args.lf:
            flower = Flower.from_json(Path(args.lf).read_text())
        else:
            flower = make_flower(args.r, args.l, args.p, args.b)

        _run(flower, args.m, args.r, args.l, args.p,
             args.b, args.sf, args.si, args.s3d, args.se3d)
        return 0
    except (OSError, ValueError, RuntimeError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    except BrokenPipeError:
        import os
        devnull = os.open(os.devnull, os.O_WRONLY)
        os.dup2(devnull, sys.stdout.fileno())
        return 1


if __name__ == "__main__":
    sys.exit(main())
