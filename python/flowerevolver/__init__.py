"""
FlowerEvolver - generates 2D/3D flowers from CPPN genomes evolved via EvoAI.

Native Python bindings (nanobind) around the same C++ core that powers the
JS/WASM API and the native desktop app -- genomes and rendered images
round-trip with both (see Flower.to_json()/from_json()).

Quick start:

    from flowerevolver import make_flower

    flower = make_flower(radius=64, num_layers=3, P=6.0, bias=1.0)
    flower.petals.image.to_png_bytes()  # -> bytes, ready to write to disk
    flower.to_json()                    # -> str, the genome
"""

from ._core import (
    DNA,
    Effects,
    Flower,
    Image,
    MutationRates,
    Petals,
    PetalsType,
    Sex,
    Stats,
    draw_flower,
    draw_petal_layer,
    draw_petals,
    get_commit_hash,
    get_flower_stats,
    get_version,
    get_version_string,
    make_3d_flower,
    make_flower,
    make_petal_layer,
    make_petals,
    make_stem,
    mutate,
    reproduce,
)

__all__ = [
    "DNA",
    "Effects",
    "Flower",
    "Image",
    "MutationRates",
    "Petals",
    "PetalsType",
    "Sex",
    "Stats",
    "draw_flower",
    "draw_petal_layer",
    "draw_petals",
    "get_commit_hash",
    "get_flower_stats",
    "get_version",
    "get_version_string",
    "make_3d_flower",
    "make_flower",
    "make_petal_layer",
    "make_petals",
    "make_stem",
    "mutate",
    "reproduce",
]

try:
    from importlib.metadata import version as _version
    __version__ = _version(__name__)
except Exception:
    __version__ = "0.0.0+unknown"
