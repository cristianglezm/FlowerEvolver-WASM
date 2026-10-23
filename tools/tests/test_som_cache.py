import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
import flowerevolver as fe
import som
from som_trainer import cache as feature_cache


@pytest.fixture()
def dataset(tmp_path):
    path = tmp_path / "dataset.json"
    assert som.main(["--generate-dataset", "6", "--out", str(path)]) == 0
    return path


@pytest.fixture()
def extractions(monkeypatch):
    """The genomes that were really rendered."""
    seen = []
    real = fe.extract_flower_features

    def counting(genome, environment):
        seen.append(genome)
        return real(genome, environment)

    monkeypatch.setattr(fe, "extract_flower_features", counting)
    return seen


def load(path):
    return som.load_dataset(path)


class TestFeatureCache:
    def test_default_path_adds_cache_to_the_name(self):
        assert feature_cache.default_path("a/dataset.json") == Path("a/dataset-cache.json")
        assert feature_cache.default_path("flowers") == Path("flowers-cache.json")

    def test_a_second_run_renders_nothing_and_gets_the_same_vectors(self, dataset, tmp_path, extractions):
        env = fe.Environment()
        cache = feature_cache.FeatureCache(tmp_path / "c.json", env)
        first = som.extract_samples(load(dataset), env, cache)
        assert len(extractions) == 6

        cache = feature_cache.FeatureCache(tmp_path / "c.json", env)
        second = som.extract_samples(load(dataset), env, cache)
        assert len(extractions) == 6
        assert second == first

    def test_only_new_flowers_are_rendered(self, dataset, tmp_path, extractions):
        env = fe.Environment()
        cache_file = tmp_path / "c.json"
        flowers = json.loads(load(dataset))["Generation"]
        som.extract_samples(json.dumps({"Generation": flowers[:4]}), env, feature_cache.FeatureCache(cache_file, env))
        assert len(extractions) == 4
        som.extract_samples(json.dumps({"Generation": flowers}), env, feature_cache.FeatureCache(cache_file, env))
        assert len(extractions) == 6

    def test_the_key_is_the_flower_not_its_position(self, dataset, tmp_path, extractions):
        env = fe.Environment()
        cache_file = tmp_path / "c.json"
        flowers = json.loads(load(dataset))["Generation"]
        som.extract_samples(json.dumps({"Generation": flowers}), env, feature_cache.FeatureCache(cache_file, env))
        shuffled = json.dumps({"Generation": list(reversed(flowers))})
        som.extract_samples(shuffled, env, feature_cache.FeatureCache(cache_file, env))
        assert len(extractions) == 6

    def test_a_flower_that_cannot_be_extracted_is_remembered_and_not_retried(self, tmp_path, extractions):
        env = fe.Environment()
        good = json.loads(som.generate_dataset(1))["Generation"][0]
        broken = {"dna": {"genomes": []}, "petals": {}}
        text = json.dumps({"Generation": [good, broken]})
        cache_file = tmp_path / "c.json"
        assert len(som.extract_samples(text, env, feature_cache.FeatureCache(cache_file, env))) == 1
        assert len(extractions) == 2
        assert len(som.extract_samples(text, env, feature_cache.FeatureCache(cache_file, env))) == 1
        assert len(extractions) == 2

    def test_another_environment_rebuilds_the_cache(self, dataset, tmp_path, extractions, capsys):
        cache_file = tmp_path / "c.json"
        env = fe.Environment()
        som.extract_samples(load(dataset), env, feature_cache.FeatureCache(cache_file, env))
        other = fe.Environment(humidity=0.9, temperature=5, altitude=100, terrain_type=1)
        cache = feature_cache.FeatureCache(cache_file, other)
        assert len(cache) == 0 and "another environment" in capsys.readouterr().err
        som.extract_samples(load(dataset), other, cache)
        assert len(extractions) == 12
        assert json.loads(cache_file.read_text())["environment"] == other.to_json()

    def test_another_layout_is_not_reused(self, dataset, tmp_path, capsys):
        env = fe.Environment()
        cache_file = tmp_path / "c.json"
        som.extract_samples(load(dataset), env, feature_cache.FeatureCache(cache_file, env))
        data = json.loads(cache_file.read_text())
        data["layout"]["input_dim"] += 1
        cache_file.write_text(json.dumps(data))
        assert len(feature_cache.FeatureCache(cache_file, env)) == 0
        assert "another feature layout" in capsys.readouterr().err

    def test_a_file_that_is_not_a_cache_is_never_overwritten(self, tmp_path):
        env = fe.Environment()
        for name, text in (("other.json", '{"Generation": []}'), ("garbage.json", "not json at all")):
            path = tmp_path / name
            path.write_text(text)
            with pytest.raises(SystemExit, match="not a feature cache"):
                feature_cache.FeatureCache(path, env)
            assert path.read_text() == text

    def test_nothing_is_written_when_nothing_changed(self, dataset, tmp_path):
        env = fe.Environment()
        cache_file = tmp_path / "c.json"
        som.extract_samples(load(dataset), env, feature_cache.FeatureCache(cache_file, env))
        before = cache_file.stat().st_mtime_ns
        som.extract_samples(load(dataset), env, feature_cache.FeatureCache(cache_file, env))
        assert cache_file.stat().st_mtime_ns == before

    def test_no_temporary_file_is_left_behind(self, dataset, tmp_path):
        env = fe.Environment()
        som.extract_samples(load(dataset), env, feature_cache.FeatureCache(tmp_path / "c.json", env))
        assert sorted(p.name for p in tmp_path.iterdir()) == ["c.json", "dataset.json"]

    def test_the_cache_is_saved_while_extracting(self, dataset, tmp_path, monkeypatch):
        env = fe.Environment()
        monkeypatch.setattr(som, "SAVE_CACHE_EVERY", 2)
        cache_file = tmp_path / "c.json"
        saved = []
        real_save = feature_cache.FeatureCache.save

        def spying(self):
            real_save(self)
            if cache_file.exists():
                saved.append(len(json.loads(cache_file.read_text())["entries"]))

        monkeypatch.setattr(feature_cache.FeatureCache, "save", spying)
        som.extract_samples(load(dataset), env, feature_cache.FeatureCache(cache_file, env))
        assert saved[:3] == [2, 4, 6]


class TestSomCliCache:
    def test_generate_dataset_with_cache_writes_the_cache_along(self, tmp_path):
        out = tmp_path / "flowers.json"
        assert som.main(["--generate-dataset", "5", "--out", str(out), "--cache"]) == 0
        cache = tmp_path / "flowers-cache.json"
        data = json.loads(cache.read_text())
        assert data["format"] == feature_cache.FORMAT and len(data["entries"]) == 5
        assert all(len(v) == fe.FlowerClassifier.input_dim for v in data["entries"].values())

    def test_generate_dataset_refuses_to_overwrite_the_cache_without_force_and_writes_nothing(self, tmp_path):
        out = tmp_path / "flowers.json"
        cache = tmp_path / "flowers-cache.json"
        cache.write_text("keep me")
        with pytest.raises(SystemExit):
            som.main(["--generate-dataset", "3", "--out", str(out), "--cache"])
        assert not out.exists() and cache.read_text() == "keep me"

    def test_generate_dataset_with_force_replaces_an_old_cache_with_one_that_matches(self, tmp_path):
        out = tmp_path / "flowers.json"
        assert som.main(["--generate-dataset", "4", "--out", str(out), "--cache"]) == 0
        old = set(json.loads((tmp_path / "flowers-cache.json").read_text())["entries"])
        assert som.main(["--generate-dataset", "4", "--out", str(out), "--cache", "--force"]) == 0
        new = set(json.loads((tmp_path / "flowers-cache.json").read_text())["entries"])
        assert len(new) == 4 and not (old & new)

    def test_an_explicit_cache_file_name(self, tmp_path):
        out, cache = tmp_path / "flowers.json", tmp_path / "somewhere.json"
        assert som.main(["--generate-dataset", "3", "--out", str(out), "--cache", str(cache)]) == 0
        assert cache.exists() and not (tmp_path / "flowers-cache.json").exists()

    def test_train_uses_the_cache_that_came_with_the_dataset(self, tmp_path, extractions):
        out = tmp_path / "flowers.json"
        assert som.main(["--generate-dataset", "6", "--out", str(out), "--cache"]) == 0
        rendered = len(extractions)
        assert som.main(["--train", "--dataset", str(out), "--cache", "--out", str(tmp_path / "fc.json"),
                         "--width", "3", "--height", "2", "--epochs", "4"]) == 0
        assert len(extractions) == rendered
        assert fe.FlowerClassifier.from_json((tmp_path / "fc.json").read_text()).width == 3

    def test_train_with_a_missing_cache_creates_it_and_finetune_reuses_it(self, tmp_path, extractions, capsys):
        out = tmp_path / "flowers.json"
        assert som.main(["--generate-dataset", "6", "--out", str(out)]) == 0
        model = tmp_path / "fc.json"
        args = ["--dataset", str(out), "--cache", "--width", "3", "--height", "2", "--epochs", "4"]
        assert som.main(["--train", *args, "--out", str(model)]) == 0
        assert (tmp_path / "flowers-cache.json").exists() and len(extractions) == 6
        capsys.readouterr()
        assert som.main(["--finetune", "--dataset", str(out), "--cache", "--load-model", str(model),
                         "--epochs", "2", "--out", str(tmp_path / "fc2.json")]) == 0
        assert len(extractions) == 6
        assert "from cache 6" in capsys.readouterr().err

    def test_cache_means_nothing_for_classify_and_export(self, tmp_path):
        with pytest.raises(SystemExit):
            som.main(["--export", "x.json", "--out", str(tmp_path / "R.hpp"), "--cache"])
