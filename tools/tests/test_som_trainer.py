import json
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
import flowerevolver as fe
from som_trainer import dataset, evaluation, export


@pytest.fixture
def flowers():
    return dataset.generate_synthetic_population(10, seed=1)


@pytest.fixture
def environment():
    return fe.Environment()


@pytest.fixture
def generation_json(flowers):
    return dataset.generation_json_from_flowers(flowers)


@pytest.fixture
def samples(flowers, environment):
    return [fe.extract_flower_features(f.to_json(), environment) for f in flowers]


@pytest.fixture
def trained_classifier(generation_json, environment):
    return fe.train_flower_classifier(generation_json, environment, fe.SOMTrainingConfig(2, 2, 20, 0.5, 1.5))


class TestDataset:
    def test_generate_synthetic_population_count_and_type(self, flowers):
        assert len(flowers) == 10
        assert all(isinstance(f, fe.Flower) for f in flowers)

    def test_num_layers_never_exceeds_what_the_radius_allows(self):
        flowers = dataset.generate_synthetic_population(120, seed=3)
        seen = set()
        for f in flowers:
            petals = json.loads(f.to_json())["Flower"]["petals"]
            assert 4 <= petals["radius"] <= 256
            assert 1 <= petals["numLayers"] <= petals["radius"].bit_length() - 1
            seen.add(petals["numLayers"])
        assert max(seen) >= 6  # deep flowers exist now (radius up to 256 allows 8 layers)

    def test_p_comes_from_the_predefined_list_with_either_sign(self):
        flowers = dataset.generate_synthetic_population(60, seed=5)
        values = [json.loads(f.to_json())["Flower"]["petals"]["P"] for f in flowers]
        assert {abs(v) for v in values} <= set(float(p) for p in dataset.DEFAULT_P_CHOICES)
        assert any(v < 0 for v in values) and any(v > 0 for v in values)
        custom = dataset.generate_synthetic_population(6, seed=5, p_choices=(7,))
        assert {abs(json.loads(f.to_json())["Flower"]["petals"]["P"]) for f in custom} == {7.0}

    def test_generate_synthetic_population_is_seed_reproducible(self):
        # genomes are not reproducible whatever the seed: make_flower() reseeds EvoAI's RNG from the clock,
        # so `seed` only controls each flower's render params.
        a = dataset.generate_synthetic_population(5, seed=7)
        b = dataset.generate_synthetic_population(5, seed=7)
        a_params = [(f.petals.radius, f.petals.num_layers, f.petals.P, f.petals.bias) for f in a]
        b_params = [(f.petals.radius, f.petals.num_layers, f.petals.P, f.petals.bias) for f in b]
        assert a_params == b_params

    def test_generation_json_from_flowers_shape(self, generation_json):
        data = json.loads(generation_json)
        assert "Generation" in data
        assert len(data["Generation"]) == 10
        for entry in data["Generation"]:
            assert "dna" in entry
            assert "petals" in entry
            assert "Flower" not in entry  # unwrapped, matching the real wire format

    def test_count_generation_entries(self, generation_json):
        assert dataset.count_generation_entries(generation_json) == 10

    def test_load_generation_json_round_trip(self, generation_json, tmp_path):
        path = tmp_path / "flowers.json"
        path.write_text(generation_json)
        assert dataset.load_generation_json(path) == generation_json

    def test_flowers_from_directory(self, flowers, tmp_path):
        for i, f in enumerate(flowers):
            (tmp_path / f"flower_{i}.json").write_text(f.to_json())
        result = dataset.flowers_from_directory(tmp_path)
        assert dataset.count_generation_entries(result) == len(flowers)

    def test_flowers_from_directory_skips_bad_files(self, flowers, tmp_path, capsys):
        (tmp_path / "good.json").write_text(flowers[0].to_json())
        (tmp_path / "bad.json").write_text("not json")
        (tmp_path / "wrong_shape.json").write_text('{"NotAFlower": {}}')
        result = dataset.flowers_from_directory(tmp_path)
        assert dataset.count_generation_entries(result) == 1
        assert "skipping" in capsys.readouterr().out


class TestEvaluation:
    def test_quantization_errors_length_and_non_negative(self, trained_classifier, samples):
        errors = evaluation.quantization_errors(trained_classifier, samples)
        assert len(errors) == len(samples)
        assert (errors >= 0).all()

    def test_mean_quantization_error_matches_manual_mean(self, trained_classifier, samples):
        errors = evaluation.quantization_errors(trained_classifier, samples)
        assert evaluation.mean_quantization_error(trained_classifier, samples) == pytest.approx(errors.mean())

    def test_node_utilization_shape_and_total(self, trained_classifier, samples):
        grid = evaluation.node_utilization(trained_classifier, samples)
        assert grid.shape == (trained_classifier.height, trained_classifier.width)
        assert grid.sum() == len(samples)

    def test_utilization_summary_keys_and_consistency(self, trained_classifier, samples):
        summary = evaluation.utilization_summary(trained_classifier, samples)
        assert summary["total_nodes"] == trained_classifier.width * trained_classifier.height
        assert summary["used_nodes"] <= summary["total_nodes"]
        assert 0.0 <= summary["fraction_used"] <= 1.0

    def test_cluster_sizes_total_equals_sample_count(self, trained_classifier, samples):
        counts = evaluation.cluster_sizes(trained_classifier, samples)
        assert sum(counts.values()) == len(samples)

    def test_training_curve_shape_and_monotonic_checkpoints(self, generation_json, environment, samples):
        def train_fn(epochs):
            return fe.train_flower_classifier(generation_json, environment, fe.SOMTrainingConfig(2, 2, epochs, 0.5, 1.5))

        curve = evaluation.training_curve(train_fn, samples, [5, 10, 20])
        assert [epoch for epoch, _ in curve] == [5, 10, 20]
        assert all(error >= 0 for _, error in curve)

    def test_training_curve_empty_checkpoints(self, generation_json, environment, samples):
        assert evaluation.training_curve(lambda e: None, samples, []) == []

    def test_one_decay_curve_reports_every_checkpoint_and_ends_at_the_final_error(self, environment, samples):
        classifier = fe.FlowerClassifier(2, 2, environment)
        curve = evaluation.training_curve_one_decay(classifier, samples, 10, 0.5, 1.5, 4)
        assert [epoch for epoch, _ in curve] == [4, 8, 10]
        assert all(error >= 0 for _, error in curve)
        assert curve[-1][1] == pytest.approx(evaluation.mean_quantization_error(classifier, samples))

    def test_one_decay_curve_trains_in_a_single_call_like_continue_training(self, environment, samples):
        # one sample makes the shuffling irrelevant, so the curve's training and a plain call must match exactly
        one = samples[:1]
        curved = fe.FlowerClassifier(2, 2, environment)
        whole = fe.FlowerClassifier.from_json(curved.to_json())
        evaluation.training_curve_one_decay(curved, one, 9, 0.5, 1.5, 3)
        whole.continue_training(one, 9, 0.5, 1.5)
        for x in range(2):
            for y in range(2):
                assert curved.get_prototype(x, y) == pytest.approx(whole.get_prototype(x, y))

    def test_training_with_reports_is_one_call_that_reports_at_the_checkpoints(self, environment, samples):
        from som_trainer.training import train_with_reports
        classifier = fe.FlowerClassifier(2, 2, environment)
        calls = []
        original = classifier.continue_training

        class Spy:  # a FlowerClassifier cannot be patched, so count through a wrapper that forwards everything
            def __getattr__(self, name):
                return getattr(classifier, name)

            def continue_training(self, *args, **kwargs):
                calls.append(kwargs.get("epoch_offset", 0))
                return original(*args, **kwargs)

        reported = []
        final = train_with_reports(Spy(), samples, 7, 0.5, 1.5, 3, lambda done, e: reported.append(done))
        assert calls == [0]
        assert reported == [3, 6, 7]
        assert final["mean_quantization_error"] == pytest.approx(classifier.evaluate(samples)["mean_quantization_error"])
        assert train_with_reports(fe.FlowerClassifier(2, 2, environment), samples, 2, 0.5, 1.5, 5) is not None  # no callback

    def test_the_epoch_curve_has_one_point_per_epoch(self, environment, samples):
        classifier = fe.FlowerClassifier(2, 2, environment)
        report = classifier.continue_training(samples, 6, 0.5, 1.5)
        curve = evaluation.epoch_curve(report)
        assert [epoch for epoch, _ in curve] == [1, 2, 3, 4, 5, 6]
        assert [d for _, d in curve] == [e.mean_distance for e in report.epochs]
        # trained, the samples are closer to their species than they were in the first epoch
        assert curve[-1][1] < curve[0][1]

    def test_one_decay_curve_rejects_a_report_interval_of_zero(self, environment, samples):
        with pytest.raises(ValueError):
            evaluation.training_curve_one_decay(fe.FlowerClassifier(2, 2, environment), samples, 4, 0.5, 1.5, 0)


class TestExport:
    def test_write_resources_hpp_contains_json_and_compiles_as_a_string_literal(self, trained_classifier, tmp_path):
        path = export.write_resources_hpp(trained_classifier.to_json(), tmp_path / "Resources.hpp")
        content = path.read_text()
        assert "namespace fe::resources" in content
        assert "const std::string trainedSOM" in content
        # the embedded literal, once unescaped, must be exactly the classifier JSON
        start = content.index('trainedSOM = "') + len('trainedSOM = "')
        end = content.rindex('";')
        literal = content[start:end]
        unescaped = literal.replace('\\"', '"').replace("\\\\", "\\")
        assert json.loads(unescaped) == json.loads(trained_classifier.to_json())

    def test_write_resources_hpp_escapes_quotes_and_backslashes(self, tmp_path):
        tricky = '{"a": "quote\\"here", "b": "back\\\\slash"}'
        path = export.write_resources_hpp(tricky, tmp_path / "Resources.hpp")
        content = path.read_text()
        assert '\\"' in content
        assert "\\\\\\\\" in content or "\\\\" in content
