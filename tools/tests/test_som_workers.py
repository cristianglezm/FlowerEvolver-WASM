"""Feature extraction in worker processes (som_trainer/parallel.py, --workers).

Extraction is not bit-for-bit reproducible between two calls (a pixel of the silhouette, a stat in the last
digits, for about 17% of the flowers), so a sample from a worker is compared with the serial one by its mean
difference, which a different flower is far above.
"""
import json
import os
import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
import flowerevolver as fe
import som
from som_trainer import cache as feature_cache, parallel

# a flower that cannot be extracted (what the serial loop skips)
UNUSABLE = {"dna": {"genomes": [{}]}, "petals": {"radius": 64, "numLayers": 3, "P": 6.0, "bias": 1.0}}
CLOSE = 0.02


def mean_difference(a, b):
    assert len(a) == len(b)
    return sum(abs(x - y) for x, y in zip(a, b)) / len(a)


def text(flowers):
    return json.dumps({"Generation": flowers})


@pytest.fixture()
def flowers(tmp_path):
    path = tmp_path / "dataset.json"
    assert som.main(["--generate-dataset", "6", "--out", str(path)]) == 0
    return json.loads(path.read_text())["Generation"]


@pytest.fixture()
def env():
    return fe.Environment()


class TestResolveWorkers:
    def test_none_and_one_are_no_pool(self):
        assert parallel.resolve_workers(None) == 1
        assert parallel.resolve_workers(1) == 1

    def test_a_number_is_that_many(self):
        assert parallel.resolve_workers(3) == 3

    def test_zero_is_one_per_core(self):
        assert parallel.resolve_workers(0) == (os.cpu_count() or 1)

    def test_a_negative_number_is_refused(self):
        with pytest.raises(ValueError, match="workers"):
            parallel.resolve_workers(-1)


class TestExtractSamplesWithAPool:
    def test_the_samples_are_the_serial_ones_in_the_same_order(self, flowers, env):
        serial = som.extract_samples(text(flowers), env)
        pooled = som.extract_samples(text(flowers), env, workers=2)
        assert len(pooled) == len(serial) == len(flowers)
        for mine, theirs in zip(pooled, serial):
            assert mean_difference(mine, theirs) < CLOSE
        # the check above can fail: the sample of the next flower is not close
        n = len(serial)
        assert any(mean_difference(pooled[i], serial[(i + 1) % n]) > CLOSE for i in range(n))

    def test_unusable_flowers_are_skipped_like_the_serial_run(self, flowers, env, capsys):
        mixed = flowers[:2] + [UNUSABLE] + flowers[2:]
        serial = som.extract_samples(text(mixed), env)
        serial_progress = capsys.readouterr().err
        pooled = som.extract_samples(text(mixed), env, workers=2)
        pooled_progress = capsys.readouterr().err
        assert len(pooled) == len(serial) == len(flowers)
        assert "skipped 1" in serial_progress and "skipped 1" in pooled_progress
        for mine, theirs in zip(pooled, serial):
            assert mean_difference(mine, theirs) < CLOSE

    def test_a_dataset_of_nothing_usable_is_still_refused(self, env):
        with pytest.raises(SystemExit, match="no usable flowers"):
            som.extract_samples(text([UNUSABLE, UNUSABLE, UNUSABLE]), env, workers=2)

    def test_one_flower_to_render_needs_no_pool(self, flowers, env, monkeypatch):
        def no_pool(*args, **kwargs):
            raise AssertionError("a pool for a single flower")

        monkeypatch.setattr(parallel, "extract_many", no_pool)
        assert len(som.extract_samples(text(flowers[:1]), env, workers=4)) == 1

    def test_the_cache_is_filled_and_then_used(self, flowers, env, tmp_path, capsys):
        cache_file = tmp_path / "cache.json"
        first = som.extract_samples(text(flowers), env, feature_cache.FeatureCache(cache_file, env), workers=2)
        capsys.readouterr()
        assert len(first) == len(flowers)
        assert len(feature_cache.FeatureCache(cache_file, env)) == len(flowers)
        second = som.extract_samples(text(flowers), env, feature_cache.FeatureCache(cache_file, env), workers=2)
        assert f"from cache {len(flowers)}" in capsys.readouterr().err
        assert second == first  # read back from the file, not rendered again

    def test_equal_flowers_are_rendered_once_with_a_cache(self, flowers, env, tmp_path, capsys):
        f = flowers
        # equal flowers in the middle: the results of the pool must not shift past them
        mixed = [f[0], f[1], f[0], f[2], f[1], f[3], f[4], f[5]]
        serial = som.extract_samples(text(mixed), env, feature_cache.FeatureCache(tmp_path / "serial.json", env))
        capsys.readouterr()
        cache = feature_cache.FeatureCache(tmp_path / "pooled.json", env)
        pooled = som.extract_samples(text(mixed), env, cache, workers=2)
        assert "from cache 2" in capsys.readouterr().err
        assert len(pooled) == len(serial) == len(mixed)
        assert len(cache) == len(flowers)
        for mine, theirs in zip(pooled, serial):
            assert mean_difference(mine, theirs) < CLOSE
        assert pooled[2] == pooled[0] and pooled[4] == pooled[1]

    def test_closing_the_stream_does_not_wait_for_the_flowers_not_started(self, flowers, env):
        jobs = [json.dumps({"Flower": f}) for f in flowers] * 4
        stream = parallel.extract_many(jobs, env, 2)
        assert len(next(stream)) == 267
        started = time.monotonic()
        stream.close()
        assert time.monotonic() - started < 10


class TestWorkersOption:
    def test_a_dataset_is_cached_with_workers(self, tmp_path):
        dataset = tmp_path / "dataset.json"
        assert som.main(["--generate-dataset", "4", "--out", str(dataset), "--cache", "--workers", "2"]) == 0
        cache = feature_cache.FeatureCache(tmp_path / "dataset-cache.json", fe.Environment())
        assert len(cache) == 4

    @pytest.mark.parametrize("argv", [
        ["--workers", "-1", "--generate-dataset", "2", "--out", "{out}", "--cache"],
        ["--workers", "two", "--generate-dataset", "2", "--out", "{out}", "--cache"],
        ["--workers", "2", "--generate-dataset", "2", "--out", "{out}"],  # nothing is rendered without --cache
        ["--workers", "2", "--export", "fc.json", "--out", "{out}"],
        ["--workers", "2", "--classify", "flower.json", "--load-model", "fc.json"],
    ])
    def test_it_is_refused_where_it_means_nothing(self, argv, tmp_path, capsys):
        out = tmp_path / "out.json"
        with pytest.raises(SystemExit) as stop:
            som.main([a.format(out=out) for a in argv])
        assert stop.value.code == 2
        assert "--workers" in capsys.readouterr().err
        assert not out.exists()  # refused before anything was written

    def test_the_default_is_no_pool(self):
        assert som.build_parser().parse_args(["--train"]).workers == 1
