"""Feature extraction in worker processes.

Extracting a flower's features takes from about 50 ms to a second, and a dataset has thousands of flowers, so
the extraction is what a training run waits for. It cannot run in threads:

* EvoAI draws its random numbers from one shared generator (``randomGen()``, a ``std::mt19937_64`` without a
  lock): two threads rendering at once would race on it.
* the binding holds the GIL while it renders, so the threads would take turns anyway.

Every worker here is a process instead: its own interpreter, its own generator, nothing shared. The workers
are started with ``spawn`` on every system, so they behave the same on Linux, macOS and Windows, and this is
an importable module (not ``som.py``, which is a script) so that they can unpickle the function they run.
A spawned worker imports the main script again, so a script that asks for workers must be a file with the
usual ``if __name__ == "__main__":`` guard (``som.py`` is one, and a notebook needs nothing); from ``python -c``
or standard input it fails with a ``BrokenProcessPool``.

The workers do not get streams of random numbers of their own: they start from the state of a fresh process.
That only matters to the part of an extraction that is not reproducible between two calls (about 17% of the
flowers differ in a pixel of the silhouette or a stat in the last digits), which never changed the species
cell of a flower.
"""
from __future__ import annotations

import multiprocessing
import os
from collections.abc import Iterator, Sequence
from concurrent.futures import ProcessPoolExecutor

import flowerevolver as fe

_environment = None


def resolve_workers(workers: int | None) -> int:
    """How many processes ``workers`` asks for: None or 1 is no pool, 0 is one per core."""
    if workers is None:
        return 1
    if workers < 0:
        raise ValueError(f"workers must be 0 (one per core) or more, not {workers}")
    if workers == 0:
        return os.cpu_count() or 1
    return workers


def _start(environment_json: str) -> None:
    # runs once in every worker: an Environment cannot be pickled, its JSON can
    global _environment
    _environment = fe.Environment.from_json(environment_json)


def _extract(flower_json: str):
    """One flower's vector, or None for one that cannot be extracted (what the serial loop skips)."""
    try:
        return fe.extract_flower_features(flower_json, _environment)
    except (ValueError, RuntimeError):
        return None


def extract_many(flowers_json: Sequence[str], environment, workers: int) -> Iterator:
    """The vector (or None) of every ``{"Flower": ...}`` text, in order, from ``workers`` processes.

    A generator: the pool is shut down when it is exhausted or closed, and a flower that was not started
    yet is not rendered after that.
    """
    pool = ProcessPoolExecutor(max_workers=min(workers, len(flowers_json)),
                               mp_context=multiprocessing.get_context("spawn"),
                               initializer=_start, initargs=(environment.to_json(),))
    try:
        yield from pool.map(_extract, flowers_json)
    finally:
        pool.shutdown(wait=False, cancel_futures=True)
