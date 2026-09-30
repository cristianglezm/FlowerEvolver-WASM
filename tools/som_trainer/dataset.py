"""
Building/loading a training set of flowers.

Two sources, meant to be used one at a time:

- generate_synthetic_population(): makes up a population with make_flower(),
  for a quick end-to-end run of this whole tool with no real data on hand.
  Nothing about a randomly-initialized CPPN population is representative of
  what players will actually grow -- treat any classifier trained on this
  as a pipeline smoke test, not a real result.
- load_generation_json() / flowers_from_directory(): real flowers, either
  already exported in the "Generation" wire format this project uses to
  move a flower collection between apps (see FlowerEvolver-frontend's
  exporter worker), or a directory of individually-saved genome .json files.
"""
from __future__ import annotations

import json
import random
from collections.abc import Sequence
from pathlib import Path

import flowerevolver as fe


# Values of P (about the number of petals) that give clearly different flowers.
DEFAULT_P_CHOICES = (1, 2, 3, 4, 5, 6, 8, 12)


def generate_synthetic_population(
    count: int,
    *,
    radius_range: tuple[int, int] = (4, 256),
    num_layers_range: tuple[int, int] = (1, 8),
    p_choices: Sequence[float] = DEFAULT_P_CHOICES,
    bias_range: tuple[float, float] = (-5.0, 5.0),
    seed: int | None = None,
) -> list[fe.Flower]:
    """Random flowers via make_flower(), varying render params per-flower
    (matches how real flowers vary -- see FlowerClassifier's own design
    notes on why extract_flower_features() renders with each flower's own
    petals config rather than a fixed one). Not a stand-in for real data,
    just enough to exercise every step of this notebook/script without
    needing an exported flower collection on hand yet.

    P is a frequency (sin(P * angle) around the flower, so about P petals). It
    can be any number, but large values (around +-360) only give a circle with
    one-pixel spines, so P is drawn from a short list of values that make
    visibly different flowers, each with a random sign, instead of a uniform
    range that would be mostly spiky circles. This is for generating training
    data only; what a game uses has nothing to do with it. The radius range is
    the whole range Petals accepts.

    `seed` only controls the render params chosen *here* (radius,
    num_layers, P, bias); it doesn't make the resulting DNA/genome content
    itself reproducible -- make_flower()'s own implementation reseeds
    EvoAI's RNG from wall-clock time internally, which is real gameplay
    randomness, not something this function can or should override.
    """
    rng = random.Random(seed)
    flowers = []
    for _ in range(count):
        radius = rng.randint(*radius_range)
        # Petals clamps the radius to 4..256 and numLayers to 1..floor(log2(radius))
        # (256 allows 8), so ask for what the radius allows instead of getting a
        # silently clamped value.
        most_layers = max(num_layers_range[0], min(
            num_layers_range[1], radius.bit_length() - 1))
        num_layers = rng.randint(num_layers_range[0], most_layers)
        p = rng.choice(p_choices) * rng.choice((-1, 1))
        bias = rng.uniform(*bias_range)
        flowers.append(fe.make_flower(radius, num_layers, p, bias))
    return flowers


def generation_json_from_flowers(flowers: list[fe.Flower]) -> str:
    """Builds the {"Generation": [...]} wire format from in-memory Flowers
    -- each entry is the same {"dna": ..., "petals": ...} shape
    flower.to_json()'s own "Flower" key wraps, just without the redundant
    per-entry wrapper (matches how a real exported collection looks)."""
    generation = [json.loads(f.to_json())["Flower"] for f in flowers]
    return json.dumps({"Generation": generation})


def load_generation_json(path: str | Path) -> str:
    """Reads an already-exported {"Generation": [...]} file from disk as-is
    (train_flower_classifier() parses it, not this function -- this just
    gets the bytes off disk)."""
    return Path(path).read_text()


def flowers_from_directory(directory: str | Path, pattern: str = "*.json") -> str:
    """Builds a {"Generation": [...]} string from a directory of
    individually-saved genome files, each shaped like flower.to_json()'s
    output ({"Flower": {"dna": ..., "petals": ...}}). Files that don't
    parse as JSON, or don't have a "Flower" key, are skipped with a
    warning printed to stdout rather than aborting the whole load --
    matches train_flower_classifier()'s own skip-and-log policy for bad
    entries, one level up."""
    generation = []
    for path in sorted(Path(directory).glob(pattern)):
        try:
            data = json.loads(path.read_text())
            generation.append(data["Flower"])
        except (json.JSONDecodeError, KeyError) as e:
            print(f"skipping {path.name}: {e}")
    return json.dumps({"Generation": generation})


def count_generation_entries(generations_json: str) -> int:
    return len(json.loads(generations_json)["Generation"])
