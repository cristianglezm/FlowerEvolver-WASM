"""A disk cache of extracted feature vectors, so a dataset is rendered once.

Extraction (about 47 ms per flower) dominates every training and fine-tuning run
over a dataset that does not change. The cache maps a flower to its vector, so a
second run, or a run with other hyper-parameters, skips the rendering.

A cache belongs to one environment and one feature layout. When either differs
it is rebuilt, never reused: the environment changes the stats part of a vector
and the layout changes its length. Extraction is not bit-for-bit reproducible
between calls, so a cached vector can differ in the last digits from a fresh
one; the species cell it leads to does not change.
"""
import hashlib
import json
import os
import sys
import tempfile
from pathlib import Path

import flowerevolver as fe

FORMAT = "flowerevolver-som-feature-cache"
VERSION = 1


def default_path(dataset_path):
    """dataset.json -> dataset-cache.json, next to it."""
    path = Path(dataset_path)
    return path.with_name(f"{path.stem}-cache{path.suffix or '.json'}")


def flower_key(flower):
    """Identifies a flower by its genome and render settings, not by its position in the file."""
    text = json.dumps(flower, sort_keys=True, separators=(",", ":"))
    return hashlib.sha1(text.encode()).hexdigest()


def _layout():
    return {
        "input_dim": fe.FlowerClassifier.input_dim,
        "silhouette_size": fe.FlowerClassifier.silhouette_size,
        "stat_names": list(fe.FlowerClassifier.stat_names()),
    }


class FeatureCache:
    """Vectors by flower key; a flower that cannot be extracted is cached as None."""

    def __init__(self, path, environment, log=None):
        self.path = Path(path)
        self.environment = environment.to_json()
        self.entries = {}
        self.dirty = False
        self._log = log or (lambda message: print(message, file=sys.stderr))
        self._load()

    def _load(self):
        if not self.path.exists():
            return
        try:
            data = json.loads(self.path.read_text())
        except (OSError, ValueError):
            data = None
        if not isinstance(data, dict) or data.get("format") != FORMAT:
            # Not ours: it may be somebody's file given by mistake, so it is never overwritten.
            raise SystemExit(f"{self.path} is not a feature cache, refusing to use or overwrite it")
        if data.get("version") != VERSION:
            self._log(f"{self.path}: cache version {data.get('version')} is not {VERSION}, rebuilding it")
        elif data.get("layout") != _layout():
            self._log(f"{self.path}: made for another feature layout, rebuilding it")
        elif data.get("environment") != self.environment:
            self._log(f"{self.path}: made for another environment, rebuilding it")
        else:
            self.entries = data.get("entries", {})
            return
        self.dirty = True  # the stale file is replaced on the next save

    def __len__(self):
        return len(self.entries)

    def __contains__(self, key):
        return key in self.entries

    def get(self, key):
        return self.entries[key]

    def put(self, key, vector):
        self.entries[key] = None if vector is None else list(vector)
        self.dirty = True

    def save(self):
        if not self.dirty:
            return
        text = json.dumps({
            "format": FORMAT,
            "version": VERSION,
            "package": fe.get_version_string(),
            "layout": _layout(),
            "environment": self.environment,
            "entries": self.entries,
        }, separators=(",", ":"))
        # a crash half-way through must not leave a truncated cache that then looks foreign
        fd, tmp = tempfile.mkstemp(dir=self.path.parent, prefix=self.path.name, suffix=".tmp")
        try:
            with os.fdopen(fd, "w") as f:
                f.write(text)
            os.replace(tmp, self.path)
        except BaseException:
            Path(tmp).unlink(missing_ok=True)
            raise
        self.dirty = False
