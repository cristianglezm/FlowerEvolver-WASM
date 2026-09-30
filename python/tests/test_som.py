import json

import pytest
from flowerevolver import (
    Environment,
    Flower,
    FlowerClassifier,
    SOMTrainingConfig,
    extract_flower_features,
    make_flower,
    train_flower_classifier,
)

RADIUS, NUM_LAYERS, P, BIAS = 64, 3, 6.0, 1.0


def a_genome(P=P):
    return make_flower(RADIUS, NUM_LAYERS, P, BIAS).to_json()


def a_generation_json(count):
    flowers = [make_flower(RADIUS, NUM_LAYERS, 2.0 + i, BIAS) for i in range(count)]
    import json
    return json.dumps({"Generation": [json.loads(f.to_json())["Flower"] for f in flowers]})


class TestEnvironment:
    def test_rejects_non_finite_humidity_but_allows_unusual_finite(self):
        with pytest.raises(ValueError):
            Environment(float("nan"), 20, 0, 0)
        with pytest.raises(ValueError):
            Environment.from_json('{"humidity": 1e300}')
        Environment(5.0, 20, 0, 0)

    def test_defaults(self):
        env = Environment()
        assert env.humidity == pytest.approx(0.5)
        assert env.temperature == 20
        assert env.altitude == 0
        assert env.terrain_type == 0

    def test_full_constructor_and_rw_fields(self):
        env = Environment(0.7, 15, 100, 2)
        assert env.humidity == pytest.approx(0.7)
        env.temperature = 25
        assert env.temperature == 25

    def test_json_round_trip(self):
        env = Environment(0.7, 15, 100, 2)
        reloaded = Environment.from_json(env.to_json())
        assert reloaded.humidity == pytest.approx(env.humidity)
        assert reloaded.temperature == env.temperature
        assert reloaded.altitude == env.altitude
        assert reloaded.terrain_type == env.terrain_type


class TestSOMTrainingConfig:
    @pytest.mark.parametrize("kwargs", [
        dict(width=0), dict(height=0), dict(epochs=0), dict(base_lr=0.0), dict(base_radius=-1.0),
    ])
    def test_rejects_degenerate_values(self, kwargs):
        args = dict(width=5, height=5, epochs=100, base_lr=0.5, base_radius=2.5)
        args.update(kwargs)
        with pytest.raises(ValueError):
            SOMTrainingConfig(**args)

    def test_from_json_rejects_negative_instead_of_wrapping(self):
        with pytest.raises(ValueError):
            SOMTrainingConfig.from_json('{"width": -1}')

    def test_defaults(self):
        cfg = SOMTrainingConfig()
        assert cfg.width == 5
        assert cfg.height == 5
        assert cfg.epochs == 100

    def test_full_constructor_and_json_round_trip(self):
        cfg = SOMTrainingConfig(width=4, height=3, epochs=50, base_lr=0.3, base_radius=2.0)
        reloaded = SOMTrainingConfig.from_json(cfg.to_json())
        assert reloaded.width == 4
        assert reloaded.height == 3
        assert reloaded.epochs == 50
        assert reloaded.base_lr == pytest.approx(0.3)
        assert reloaded.base_radius == pytest.approx(2.0)


class TestExtractFlowerFeatures:
    def test_dimension_and_range(self):
        features = extract_flower_features(a_genome(), Environment())
        assert len(features) == FlowerClassifier.input_dim == 267
        assert all(0.0 <= v <= 1.0 for v in features)

    def test_raises_runtime_error_on_malformed_json(self):
        with pytest.raises(RuntimeError):
            extract_flower_features("not json", Environment())

    def test_raises_value_error_on_unusable_dna(self):
        bad = '{"Flower": {"dna": {"genomes": [{}]}, "petals": {"radius": 64, "numLayers": 3, "P": 6.0, "bias": 1.0}}}'
        with pytest.raises(ValueError):
            extract_flower_features(bad, Environment())


class TestFlowerClassifier:
    def test_construction(self):
        classifier = FlowerClassifier(4, 3, Environment())
        assert classifier.width == 4
        assert classifier.height == 3
        assert classifier.input_dim == 267

    def test_json_round_trip(self):
        classifier = FlowerClassifier(3, 3, Environment(0.7, 15, 100, 2))
        reloaded = FlowerClassifier.from_json(classifier.to_json())
        assert reloaded.width == classifier.width
        assert reloaded.height == classifier.height
        assert reloaded.environment.temperature == 15

    def test_stat_names_fixed_order(self):
        assert FlowerClassifier.stat_names() == [
            "health", "stamina", "minTemperature", "maxTemperature", "maturationPeriod",
            "toxicityRate", "vitality", "agility", "intelligence", "strength", "luck",
        ]

    def test_classify_genome_and_flower_overloads_agree(self):
        env = Environment()
        classifier = train_flower_classifier(a_generation_json(6), env, SOMTrainingConfig(2, 2, 20, 0.5, 1.5))
        flower = make_flower(RADIUS, NUM_LAYERS, 3.0, BIAS)
        via_genome = classifier.classify(flower.to_json(), env)
        via_flower = classifier.classify(flower, env)
        # the same flower, the same node; the distances only agree to the noise of the extraction
        assert (via_genome.x, via_genome.y) == (via_flower.x, via_flower.y)
        assert via_genome.distance == pytest.approx(via_flower.distance, abs=0.5)
        assert via_genome.x < classifier.width
        assert via_genome.y < classifier.height

    def test_classify_returns_none_instead_of_raising_on_bad_genome(self):
        classifier = FlowerClassifier(2, 2, Environment())
        assert classifier.classify("not json", Environment()) is None

    def test_get_prototype(self):
        classifier = FlowerClassifier(2, 2, Environment())
        proto = classifier.get_prototype(0, 0)
        assert len(proto) == FlowerClassifier.input_dim

    def test_get_prototype_out_of_range_raises_index_error(self):
        classifier = FlowerClassifier(2, 2, Environment())
        with pytest.raises(IndexError):
            classifier.get_prototype(5, 5)

    def test_classify_features_matches_classify_genome(self):
        env = Environment()
        classifier = train_flower_classifier(a_generation_json(6), env, SOMTrainingConfig(2, 2, 20, 0.5, 1.5))
        genome = a_genome(P=3.0)
        via_genome = classifier.classify(genome, env)
        via_features = classifier.classify_features(extract_flower_features(genome, env))
        assert (via_genome.x, via_genome.y) == (via_features.x, via_features.y)
        assert via_genome.distance == pytest.approx(via_features.distance, abs=0.5)

    def test_a_match_has_the_distance_to_the_prototype_of_its_node(self):
        import math
        classifier = FlowerClassifier(3, 2, Environment())
        # a prototype is its own best match, at distance 0
        at = classifier.classify_features(classifier.get_prototype(2, 1))
        assert (at.x, at.y, at.distance) == (2, 1, 0.0)
        # 0.1 away in one value is a distance of at most 0.1 from some node, the euclidean one
        moved = classifier.get_prototype(2, 1)
        moved[5] += 0.1
        near = classifier.classify_features(moved)
        proto = classifier.get_prototype(near.x, near.y)
        assert near.distance <= 0.1 + 1e-12
        assert near.distance == pytest.approx(math.sqrt(sum((a - b) ** 2 for a, b in zip(moved, proto))))
        assert "distance=" in repr(near)

    def test_classify_features_returns_none_on_wrong_size(self):
        classifier = FlowerClassifier(2, 2, Environment())
        assert classifier.classify_features([0.0, 1.0]) is None


class TestWeightDecimals:
    def test_default_is_exact_and_it_can_be_set_and_cleared(self):
        c = FlowerClassifier(3, 2, Environment())
        assert c.weight_decimals is None
        c.weight_decimals = 4
        assert c.weight_decimals == 4
        c.weight_decimals = None
        assert c.weight_decimals is None

    @pytest.mark.parametrize("bad", [-1, 18])
    def test_rejects_decimals_outside_0_to_17(self, bad):
        c = FlowerClassifier(3, 2, Environment())
        with pytest.raises(ValueError):
            c.weight_decimals = bad
        assert c.weight_decimals is None

    def test_rounds_only_the_json_and_makes_it_shorter(self):
        c = FlowerClassifier(8, 6, Environment())
        exact = c.to_json()
        c.weight_decimals = 4
        short = c.to_json()
        assert len(short) * 2 < len(exact)
        assert c.to_json() == short
        c.weight_decimals = None
        assert c.to_json() == exact  # the classifier kept its exact weights

    def test_a_loaded_classifier_holds_the_rounded_weights_and_writes_exact_ones_again(self):
        c = FlowerClassifier(4, 3, Environment())
        c.weight_decimals = 4
        loaded = FlowerClassifier.from_json(c.to_json())
        assert loaded.weight_decimals is None
        for y in range(3):
            for x in range(4):
                for before, after in zip(c.get_prototype(x, y), loaded.get_prototype(x, y)):
                    assert after == pytest.approx(before, abs=5.1e-5)


class TestTrainFlowerClassifier:
    def test_raises_when_no_sample_is_usable(self):
        cfg = SOMTrainingConfig(3, 3, 5, 0.5, 1.5)
        with pytest.raises(ValueError):
            train_flower_classifier([[1.0, 2.0], [3.0]], Environment(), cfg)

    def test_from_samples(self):
        env = Environment()
        samples = [extract_flower_features(a_genome(P=2.0 + i), env) for i in range(6)]
        classifier = train_flower_classifier(samples, env, SOMTrainingConfig(2, 2, 20, 0.5, 1.5))
        assert classifier.width == 2
        assert classifier.height == 2
        for sample in samples:
            match = classifier.classify_features(sample)
            assert match is not None
            assert match.x < 2 and match.y < 2
            assert match.distance >= 0

    def test_continue_training_further_reduces_or_maintains_quantization_error(self):
        env = Environment()
        samples = [extract_flower_features(a_genome(P=2.0 + i), env) for i in range(8)]
        classifier = train_flower_classifier(samples, env, SOMTrainingConfig(2, 2, 5, 0.5, 1.5))

        def mean_qerror():
            return sum(classifier.classify_features(s).distance for s in samples) / len(samples)

        before = mean_qerror()
        report = classifier.continue_training(samples, epochs=30, base_lr=0.3, base_radius=1.0)
        after = mean_qerror()
        assert report.skipped == []
        assert after <= before + 1e-9  # shouldn't get meaningfully worse

    @pytest.mark.parametrize("bad", [0.0, -1.0, float("nan"), float("inf")])
    def test_continue_training_rejects_bad_base_lr_or_radius(self, bad):
        env = Environment()
        samples = [extract_flower_features(a_genome(), env)]
        classifier = train_flower_classifier(samples, env, SOMTrainingConfig(2, 2, 5, 0.5, 1.5))
        with pytest.raises(ValueError):
            classifier.continue_training(samples, epochs=5, base_lr=bad, base_radius=1.0)
        with pytest.raises(ValueError):
            classifier.continue_training(samples, epochs=5, base_lr=0.5, base_radius=bad)

    def test_continue_training_raises_when_no_sample_is_usable(self):
        env = Environment()
        samples = [extract_flower_features(a_genome(), env)]
        classifier = train_flower_classifier(samples, env, SOMTrainingConfig(2, 2, 5, 0.5, 1.5))
        before = classifier.to_json()
        with pytest.raises(ValueError):
            classifier.continue_training([], epochs=5, base_lr=0.5, base_radius=1.0)
        with pytest.raises(ValueError):
            classifier.continue_training([[1.0], [2.0, 3.0]], epochs=5, base_lr=0.5, base_radius=1.0)
        assert classifier.to_json() == before

    def test_validate_methods(self):
        Environment().validate()
        SOMTrainingConfig().validate()
        cfg = SOMTrainingConfig()
        cfg.width = 0
        with pytest.raises(ValueError):
            cfg.validate()
        env = Environment()
        env.humidity = float("inf")
        with pytest.raises(ValueError):
            env.validate()

    def test_environment_changed_after_construction_is_rejected_at_use(self):
        env = Environment()
        env.humidity = float("nan")
        with pytest.raises(ValueError):
            extract_flower_features(a_genome(), env)

    def test_config_changed_after_construction_is_rejected_at_use(self):
        env = Environment()
        samples = [extract_flower_features(a_genome(), env)]
        cfg = SOMTrainingConfig(2, 2, 5, 0.5, 1.5)
        cfg.base_lr = 0.0
        with pytest.raises(ValueError):
            train_flower_classifier(samples, env, cfg)

    def test_evaluate_matches_a_hand_computation(self):
        import math
        env = Environment()
        samples = [extract_flower_features(a_genome(), env) for _ in range(6)]
        classifier = train_flower_classifier(samples, env, SOMTrainingConfig(3, 2, 5, 0.5, 1.5))
        e = classifier.evaluate(samples + [[0.5, 0.5]])
        counts = [0] * 6
        total = 0.0
        for s in samples:
            m = classifier.classify_features(s)
            proto = classifier.get_prototype(m.x, m.y)
            by_hand = math.sqrt(sum((a - b) ** 2 for a, b in zip(s, proto)))
            assert m.distance == pytest.approx(by_hand)
            total += by_hand
            counts[m.y * 3 + m.x] += 1
        assert e["node_counts"] == counts
        assert e["samples_used"] == 6 and e["samples_skipped"] == 1
        assert e["mean_quantization_error"] == pytest.approx(total / 6)
        assert e["used_nodes"] == sum(1 for c in counts if c)
        assert e["busiest_node_count"] == max(counts)
        with pytest.raises(ValueError):
            classifier.evaluate([[0.5, 0.5]])

    def test_classify_flowers_one_entry_per_flower_in_order(self):
        env = Environment()
        genomes = [a_genome() for _ in range(3)]
        samples = [extract_flower_features(g, env) for g in genomes]
        classifier = train_flower_classifier(samples, env, SOMTrainingConfig(3, 2, 5, 0.5, 1.5))
        flowers = [json.loads(g)["Flower"] for g in genomes]
        flowers.insert(1, {})
        matches = classifier.classify_flowers(json.dumps({"Generation": flowers}), env)
        assert len(matches) == 4
        assert matches[1] is None
        assert all(matches[i] is not None for i in (0, 2, 3))
        with pytest.raises(RuntimeError):
            classifier.classify_flowers("not json", env)

    def test_continue_training_reports_every_epoch_and_the_skipped_samples(self):
        env = Environment()
        samples = [extract_flower_features(a_genome(P=2.0 + i), env) for i in range(6)]
        samples.insert(2, [1.0, 2.0])  # wrong size
        classifier = FlowerClassifier(3, 2, env)
        report = classifier.continue_training(samples, epochs=5, base_lr=0.5, base_radius=1.5)
        assert report.skipped == [2]
        assert [e.epoch for e in report.epochs] == [0, 1, 2, 3, 4]
        assert all(e.mean_distance >= 0 for e in report.epochs)
        # the decay ends at its floors, 0.01 and 0.5, in the last epoch
        assert report.epochs[-1].learning_rate == pytest.approx(0.01)
        assert report.epochs[-1].radius == pytest.approx(0.5)
        assert sum(report.node_counts) == 6 and len(report.node_counts) == 6
        assert "mean_distance=" in repr(report.epochs[0]) and "epochs=5" in repr(report)

    def test_continue_training_numbers_the_epochs_with_the_offset(self):
        env = Environment()
        sample = extract_flower_features(a_genome(), env)
        classifier = FlowerClassifier(2, 2, env)
        report = classifier.continue_training([sample], 2, 0.5, 1.5, epoch_offset=3, total_epochs=5)
        assert [e.epoch for e in report.epochs] == [3, 4]

    def test_on_epoch_sees_every_epoch_and_false_stops_it(self):
        env = Environment()
        sample = extract_flower_features(a_genome(), env)
        classifier = FlowerClassifier(2, 2, env)
        seen = []

        def stop_after_three(epoch):
            seen.append(epoch.epoch)
            return epoch.epoch < 2  # False from the third epoch on

        report = classifier.continue_training([sample], 20, 0.5, 1.5, on_epoch=stop_after_three)
        assert seen == [0, 1, 2]
        assert len(report.epochs) == 3
        # None (a callback that returns nothing) goes on
        seen.clear()
        report = classifier.continue_training([sample], 4, 0.5, 1.5, on_epoch=lambda e: seen.append(e.epoch))
        assert seen == [0, 1, 2, 3] and len(report.epochs) == 4

    def test_on_epoch_exceptions_and_non_callables_are_raised(self):
        env = Environment()
        sample = extract_flower_features(a_genome(), env)
        classifier = FlowerClassifier(2, 2, env)

        def boom(epoch):
            raise RuntimeError("stop here")

        with pytest.raises(RuntimeError, match="stop here"):
            classifier.continue_training([sample], 5, 0.5, 1.5, on_epoch=boom)
        with pytest.raises(TypeError):
            classifier.continue_training([sample], 5, 0.5, 1.5, on_epoch=3)

    def test_continue_training_can_split_one_decay(self):
        env = Environment()
        sample = extract_flower_features(a_genome(), env)
        cfg = SOMTrainingConfig(3, 2, 5, 0.5, 1.5)
        a = train_flower_classifier([sample], env, cfg)
        b = FlowerClassifier.from_json(a.to_json())
        c = FlowerClassifier.from_json(a.to_json())
        a.continue_training([sample], 20, 0.5, 1.5)
        b.continue_training([sample], 10, 0.5, 1.5, epoch_offset=0, total_epochs=20)
        b.continue_training([sample], 10, 0.5, 1.5, epoch_offset=10, total_epochs=20)
        c.continue_training([sample], 10, 0.5, 1.5)
        c.continue_training([sample], 10, 0.5, 1.5)
        def proto(x):
            return [v for y in range(2) for xx in range(3) for v in x.get_prototype(xx, y)]
        assert max(abs(p - q) for p, q in zip(proto(a), proto(b))) < 1e-9
        assert max(abs(p - q) for p, q in zip(proto(a), proto(c))) > 1e-3
        with pytest.raises(ValueError):
            a.continue_training([sample], 10, 0.5, 1.5, epoch_offset=15, total_epochs=20)

    def test_from_generations_json(self):
        classifier = train_flower_classifier(a_generation_json(5), Environment(), SOMTrainingConfig(2, 2, 10, 0.5, 1.5))
        assert classifier.width == 2
        assert classifier.height == 2

    def test_from_generations_json_skips_bad_entries(self, capsys):
        import json
        good = json.loads(make_flower(RADIUS, NUM_LAYERS, P, BIAS).to_json())["Flower"]
        bad = {"dna": {"genomes": [{}]}, "petals": {"radius": 64, "numLayers": 3, "P": 6.0, "bias": 1.0}}
        generations_json = json.dumps({"Generation": [good, bad]})

        classifier = train_flower_classifier(generations_json, Environment(), SOMTrainingConfig(2, 2, 5, 0.5, 1.5))
        assert classifier.width == 2

    def test_from_generations_json_raises_runtime_error_on_malformed_json(self):
        with pytest.raises(RuntimeError):
            train_flower_classifier("not json", Environment(), SOMTrainingConfig())
