#include <catch2/catch_test_macros.hpp>

#include <fe/Flower.hpp>

#include "test_helpers.hpp"

namespace {
    bool hasAnyNonTransparentPixel(const fe::Image& img){
        for(std::size_t i = 3; i < img.imageData.size(); i += 4){
            if(img.imageData[i] != 0){
                return true;
            }
        }
        return false;
    }
}

TEST_CASE("default constructor is empty", "[Flower]"){
    fe::Flower f;
    REQUIRE(f.dna.size() == 0u);
}

TEST_CASE("random-flower constructor builds a 2-genome DNA (stats genome + petals CPPN)", "[Flower]"){
    fe::Flower f({0.f, 0.f}, 64, 3, 6.0f, 1.0f);
    REQUIRE(f.dna.size() == 2u);
    auto size = f.petals.image.getSize();
    REQUIRE(size.x == 64 * 2);
    REQUIRE(size.y == 64 * 3);
}

TEST_CASE("random-flower constructor actually paints something (not left fully transparent)", "[Flower]"){
    fe::Flower f({0.f, 0.f}, 32, 2, 6.0f, 1.0f);
    REQUIRE(hasAnyNonTransparentPixel(f.petals.image));
}

TEST_CASE("constructing from an existing DNA requires at least 2 genomes", "[Flower]"){
    fe::DNA dna;
    dna.add(EvoAI::Genome(4, 14, false, true)); // only the stats genome, missing the petals CPPN
    REQUIRE_THROWS_AS(
        fe::Flower({0.f, 0.f}, 64, 3, 6.0f, 1.0f, std::move(dna)),
        std::invalid_argument
    );
}

TEST_CASE("constructing from a valid 2-genome DNA succeeds and reuses that DNA", "[Flower]"){
    fe::DNA seed;
    seed.add(EvoAI::Genome(4, 14, false, true));
    seed.add(EvoAI::Genome(4, 4, false, true));

    fe::Flower f({0.f, 0.f}, 64, 3, 6.0f, 1.0f, std::move(seed));
    REQUIRE(f.dna.size() == 2u);
}

TEST_CASE("constructing from a DNA whose petals genome has fewer than 4 real outputs throws", "[Flower]"){
    fe::DNA seed;
    seed.add(EvoAI::Genome(4, 14, false, true));
    seed.add(EvoAI::Genome(4, 3, false, true));
    REQUIRE_THROWS_AS(
        fe::Flower({0.f, 0.f}, 64, 3, 6.0f, 1.0f, std::move(seed)),
        std::invalid_argument
    );
}

TEST_CASE("copy constructor deep-copies the DNA", "[Flower]"){
    fe::Flower original({0.f, 0.f}, 32, 2, 6.0f, 1.0f);
    fe::Flower copy(original);
    REQUIRE(copy.dna.size() == original.dna.size());

    copy.dna.setFitness(42.0);
    REQUIRE(original.dna.getFitness() != 42.0);
}

TEST_CASE("toJson / Flower(JsonBox::Object) round-trips the genome count", "[Flower]"){
    fe::Flower original({0.f, 0.f}, 32, 2, 6.0f, 1.0f);
    JsonBox::Object o = original.toJson().getObject();
    fe::Flower fromJson(o);
    REQUIRE(fromJson.dna.size() == original.dna.size());
}

TEST_CASE("Flower(JsonBox::Object) rejects a DNA with fewer than 2 genomes", "[Flower]"){
    JsonBox::Value v;
    v.loadFromString(R"({"dna":{"genomes":[{}]},"petals":{}})"); // 1 genome, not 2
    REQUIRE_THROWS_AS(fe::Flower(v.getObject()), std::invalid_argument);
}

TEST_CASE("Flower(JsonBox::Object) rejects genomes with 2 entries but no actual node chromosomes", "[Flower]"){
    JsonBox::Value v;
    v.loadFromString(R"({"dna":{"genomes":[{},{}]},"petals":{}})");
    REQUIRE_THROWS_AS(fe::Flower(v.getObject()), std::invalid_argument);
}

TEST_CASE("drawing the same DNA from the same RNG state is deterministic", "[Flower]"){
    EvoAI::randomGen().setSeed(12345);
    fe::DNA seed;
    seed.add(EvoAI::Genome(4, 14, false, true));
    seed.add(EvoAI::Genome(4, 4, false, true));
    fe::DNA seedCopy(seed);

    EvoAI::randomGen().setSeed(12345);
    fe::Flower a({0.f, 0.f}, 40, 2, 6.0f, 1.0f, std::move(seed));

    EvoAI::randomGen().setSeed(12345);
    fe::Flower b({0.f, 0.f}, 40, 2, 6.0f, 1.0f, std::move(seedCopy));

    fe_tests::requireBytesEqual(a.petals.image.imageData, b.petals.image.imageData);
}
