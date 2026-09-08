import json
import pytest
from flowerevolver import (
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
    get_flower_stats,
    make_3d_flower,
    make_flower,
    make_petal_layer,
    make_petals,
    make_stem,
    mutate,
    reproduce,
)

RADIUS, NUM_LAYERS, P, BIAS = 32, 2, 6.0, 1.0


def a_flower():
    return make_flower(RADIUS, NUM_LAYERS, P, BIAS)


class TestMutationRates:
    def test_defaults_match_documented_values(self):
        mr = MutationRates()
        assert mr.add_node_rate == pytest.approx(0.2)
        assert mr.add_conn_rate == pytest.approx(0.3)
        assert mr.remove_conn_rate == pytest.approx(0.2)
        assert mr.perturb_weights_rate == pytest.approx(0.6)
        assert mr.enable_rate == pytest.approx(0.35)
        assert mr.disable_rate == pytest.approx(0.3)
        assert mr.act_type_rate == pytest.approx(0.4)

    def test_full_constructor_and_rw_fields(self):
        mr = MutationRates(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)
        assert mr.add_node_rate == pytest.approx(0.1)
        mr.add_node_rate = 0.9
        assert mr.add_node_rate == pytest.approx(0.9)


class TestDNA:
    def test_starts_empty(self):
        dna = DNA()
        assert dna.size() == 0
        assert len(dna) == 0
        assert dna.get_fitness() == pytest.approx(0.0)

    def test_make_flower_produces_two_genomes(self):
        flower = a_flower()
        assert flower.dna.size() == 2
        assert len(flower.dna) == 2

    def test_copy_constructor_is_independent(self):
        original = a_flower().dna
        copy = DNA(original)
        copy.set_fitness(42.0)
        assert original.get_fitness() != pytest.approx(42.0)
        assert copy.get_fitness() == pytest.approx(42.0)

    def test_set_fitness_get_fitness_round_trip(self):
        dna = a_flower().dna
        dna.set_fitness(3.5)
        assert dna.get_fitness() == pytest.approx(3.5)

    def test_to_json_from_json_round_trips_size(self):
        dna = a_flower().dna
        restored = DNA.from_json(dna.to_json())
        assert restored.size() == dna.size()

    def test_from_json_rejects_malformed_json(self):
        with pytest.raises(RuntimeError):
            DNA.from_json("not json")

    def test_from_json_rejects_non_object_json(self):
        with pytest.raises(ValueError):
            DNA.from_json("[1, 2, 3]")

    def test_reproduce_requires_matching_genome_counts(self):
        a = a_flower().dna
        with pytest.raises(RuntimeError):
            DNA.reproduce(a, DNA())

    def test_reproduce_of_matching_dna_yields_same_size_child(self):
        a, b = a_flower().dna, a_flower().dna
        child = DNA.reproduce(a, b)
        assert child.size() == a.size()

    def test_distance_requires_matching_genome_counts(self):
        a = a_flower().dna
        with pytest.raises(RuntimeError):
            DNA.distance(a, DNA())

    def test_distance_of_dna_against_itself_is_zero(self):
        a = a_flower().dna
        b = DNA(a)
        assert DNA.distance(a, b) == pytest.approx(0.0, abs=1e-4)

    def test_mutate_does_not_change_genome_count(self):
        dna = a_flower().dna
        dna.mutate(MutationRates())
        assert dna.size() == 2

    def test_repr_does_not_raise(self):
        assert "DNA(" in repr(a_flower().dna)


class TestImage:
    def test_create_allocates_transparent_buffer(self):
        img = Image()
        img.create(4, 3, (0, 0, 0, 0))
        assert img.width == 4
        assert img.height == 3
        assert img.get_pixel(0, 0) == (0, 0, 0, 0)

    def test_set_pixel_get_pixel_round_trip(self):
        img = Image()
        img.create(2, 2, (0, 0, 0, 0))
        img.set_pixel(1, 0, (10, 20, 30, 255))
        assert img.get_pixel(1, 0) == (10, 20, 30, 255)
        # untouched pixel stays whatever create() set it to
        assert img.get_pixel(0, 0) == (0, 0, 0, 0)

    def test_get_pixel_out_of_range_is_bounds_safe(self):
        img = Image()
        img.create(2, 2, (0, 0, 0, 0))
        # out-of-range returns fe::Color()'s default - opaque black, not
        # transparent - matches tests/cpp/test_Image.cpp's own
        # `REQUIRE(px == fe::Color())` expectation.
        assert img.get_pixel(50, 50) == (0, 0, 0, 255)

    def test_get_pixel_does_not_alias_x_equals_width_into_next_row(self):
        img = Image()
        img.create(4, 4, (0, 0, 0, 0))
        img.set_pixel(0, 1, (9, 9, 9, 9))
        assert img.get_pixel(4, 0) == (0, 0, 0, 255)

    def test_to_png_bytes_produces_a_well_formed_png(self):
        img = Image()
        img.create(8, 8, (255, 255, 255, 255))
        png = img.to_png_bytes()
        assert isinstance(png, bytes)
        assert png[:8] == b"\x89PNG\r\n\x1a\n"

    def test_rendered_flower_image_has_documented_dimensions(self):
        flower = a_flower()
        assert flower.petals.image.width == RADIUS * 2
        assert flower.petals.image.height == RADIUS * 3


class TestPetals:
    def test_defaults(self):
        p = Petals()
        assert p.radius == 64
        assert p.num_layers == 3
        assert p.P == pytest.approx(6.0)
        assert p.bias == pytest.approx(1.0)
        assert p.has_bloom is False

    def test_sized_constructor_clamps_radius(self):
        assert Petals(1, 1, 6.0, 1.0).radius == 4
        assert Petals(9999, 1, 6.0, 1.0).radius == 256

    def test_type_enum_has_three_members(self):
        assert {PetalsType.TRUNK, PetalsType.PETALS, PetalsType.TRUNK_AND_PETALS} == set(
            PetalsType.__members__.values())


class TestFlower:
    def test_make_flower_has_random_petals(self):
        flower = a_flower()
        w, h = flower.petals.image.width, flower.petals.image.height
        assert any(
            flower.petals.image.get_pixel(x, y)[3] != 0
            for x in range(0, w, 4)
            for y in range(0, h, 4)
        )

    def test_to_json_round_trips_via_from_json(self):
        flower = a_flower()
        restored = Flower.from_json(flower.to_json())
        assert restored.dna.size() == flower.dna.size()

    def test_to_json_matches_the_documented_wire_shape(self):
        flower = a_flower()
        parsed = json.loads(flower.to_json())
        assert "Flower" in parsed
        assert "dna" in parsed["Flower"]
        assert "petals" in parsed["Flower"]
        assert len(parsed["Flower"]["dna"]["genomes"]) == 2

    def test_from_json_rejects_malformed_json(self):
        with pytest.raises(RuntimeError):
            Flower.from_json("not json")

    def test_from_json_rejects_missing_dna(self):
        with pytest.raises(ValueError):
            Flower.from_json("{}")

    def test_from_json_rejects_dna_with_fewer_than_two_genomes(self):
        with pytest.raises(ValueError):
            Flower.from_json('{"Flower":{"dna":{"genomes":[]},"petals":{}}}')

    def test_repr_does_not_raise(self):
        assert "Flower(" in repr(a_flower())


class TestApi:
    def test_make_petals_makes_petals_returns_two_genome_flower(self):
        assert make_petals(RADIUS, NUM_LAYERS, P, BIAS).dna.size() == 2

    def test_make_stem_returns_two_genome_flower(self):
        assert make_stem(RADIUS, NUM_LAYERS, P, BIAS).dna.size() == 2

    def test_make_petal_layer_returns_a_flower(self):
        assert make_petal_layer(RADIUS, NUM_LAYERS, P, BIAS, 0).dna.size() == 2

    def test_draw_flower_renders_a_correctly_sized_image(self):
        flower = a_flower()
        img = draw_flower(flower.dna, RADIUS, NUM_LAYERS, P, BIAS)
        assert img.width == flower.petals.image.width
        assert img.height == flower.petals.image.height

    def test_draw_flower_rejects_dna_with_fewer_than_two_genomes(self):
        with pytest.raises(ValueError):
            draw_flower(DNA(), RADIUS, NUM_LAYERS, P, BIAS)

    def test_draw_petals_and_draw_petal_layer_reject_empty_dna(self):
        with pytest.raises(ValueError):
            draw_petals(DNA(), RADIUS, NUM_LAYERS, P, BIAS)
        with pytest.raises(ValueError):
            draw_petal_layer(DNA(), RADIUS, NUM_LAYERS, P, BIAS, 0)

    def test_reproduce_of_two_valid_parents_yields_two_genome_child(self):
        father, mother = a_flower(), a_flower()
        child = reproduce(father.dna, mother.dna, RADIUS, NUM_LAYERS, P, BIAS)
        assert child.dna.size() == 2

    def test_reproduce_requires_matching_genome_counts(self):
        father = a_flower()
        with pytest.raises(RuntimeError):
            reproduce(father.dna, DNA(), RADIUS, NUM_LAYERS, P, BIAS)

    def test_mutate_uses_documented_default_rates_and_leaves_input_untouched(self):
        original = a_flower()
        before_json = original.dna.to_json()
        mutated = mutate(original.dna, RADIUS, NUM_LAYERS, P, BIAS)
        assert mutated.dna.size() == 2
        assert original.dna.to_json() == before_json

    def test_mutate_accepts_custom_rates(self):
        original = a_flower()
        mutated = mutate(
            original.dna, RADIUS, NUM_LAYERS, P, BIAS,
            add_node_rate=0.1, add_conn_rate=0.1, remove_conn_rate=0.1,
            perturb_weights_rate=0.1, enable_rate=0.1, disable_rate=0.1,
            act_type_rate=0.1,
        )
        assert mutated.dna.size() == 2

    def test_mutate_rejects_dna_with_fewer_than_two_genomes(self):
        with pytest.raises(ValueError):
            mutate(DNA(), RADIUS, NUM_LAYERS, P, BIAS)

    def test_get_flower_stats_returns_values_in_documented_ranges(self):
        flower = a_flower()
        stats = get_flower_stats(flower.to_json(), 0.5, 20, 0, 0)
        assert isinstance(stats, Stats)
        assert 1 <= stats.health <= 100
        assert 1 <= stats.stamina <= 100
        assert stats.min_temperature <= stats.max_temperature
        assert stats.sex in (Sex.MALE, Sex.FEMALE, Sex.BOTH)
        effects = stats.effects
        assert isinstance(effects, Effects)
        for value in (effects.vitality, effects.agility, effects.intelligence,
                      effects.strength, effects.luck):
            assert -100.0 <= value <= 100.0

    def test_get_flower_stats_rejects_malformed_genome(self):
        with pytest.raises(RuntimeError):
            get_flower_stats("not json", 0.5, 20, 0, 0)

    def test_get_flower_stats_rejects_dna_with_fewer_than_two_genomes(self):
        with pytest.raises(ValueError):
            get_flower_stats(
                '{"Flower":{"dna":{"genomes":[{}]}}}', 0.5, 20, 0, 0)

    def test_make_3d_flower_returns_a_well_formed_gltf_document(self):
        flower = a_flower()
        gltf = make_3d_flower(flower.dna, RADIUS,
                              NUM_LAYERS, P, BIAS, "test-flower")
        parsed = json.loads(gltf)
        assert parsed["asset"]["version"] == "2.0"
        assert "nodes" in parsed
        assert "meshes" in parsed

    def test_make_3d_flower_rejects_invalid_params(self):
        flower = a_flower()
        with pytest.raises(ValueError):
            make_3d_flower(flower.dna, 0, NUM_LAYERS, P, BIAS, "id")
        with pytest.raises(ValueError):
            make_3d_flower(flower.dna, RADIUS, NUM_LAYERS, P, BIAS, "")
