#include <catch2/catch_test_macros.hpp>

#include <fe/FlowerEvolver.hpp>

namespace {
    bool hasAnyNonTransparentPixel(const fe::Image& img){
        for(std::size_t i = 3; i < img.imageData.size(); i += 4){
            if(img.imageData[i] != 0){
                return true;
            }
        }
        return false;
    }
    fe::DNA emptyDna(){
        return fe::DNA();
    }
    fe::DNA oneGenomeDna(){
        fe::DNA dna;
        dna.add(EvoAI::Genome(4, 14, false, true));
        return dna;
    }
}

TEST_CASE("makeFlower returns a 2-genome Flower with a painted image", "[NativeAPI]"){
    auto flower = makeFlower(64, 3, 6.0f, 1.0f);
    REQUIRE(flower.dna.size() == 2u);
    REQUIRE(hasAnyNonTransparentPixel(flower.petals.image));
}

TEST_CASE("makePetals / makeStem also return a 2-genome Flower", "[NativeAPI]"){
    auto petals = makePetals(64, 3, 6.0f, 1.0f);
    REQUIRE(petals.dna.size() == 2u);

    auto stem = makeStem(64, 3, 6.0f, 1.0f);
    REQUIRE(stem.dna.size() == 2u);
    REQUIRE(hasAnyNonTransparentPixel(stem.petals.image));
}

TEST_CASE("makePetalLayer returns a Flower with a 2-genome DNA (stats genome + reused CPPN)", "[NativeAPI]"){
    auto layer = makePetalLayer(64, 3, 6.0f, 1.0f, 0);
    REQUIRE(layer.dna.size() == 2u);
}

TEST_CASE("drawFlower renders a correctly-sized, non-empty image for a given DNA", "[NativeAPI]"){
    auto flower = makeFlower(40, 2, 6.0f, 1.0f);
    auto redrawn = drawFlower(flower.dna, 40, 2, 6.0f, 1.0f);

    REQUIRE(redrawn.imageData.size() == flower.petals.image.imageData.size());
    bool hasContent = false;
    for(std::size_t i = 3; i < redrawn.imageData.size(); i += 4){
        if(redrawn.imageData[i] != 0){ hasContent = true; break; }
    }
    REQUIRE(hasContent);
}

TEST_CASE("drawFlower/drawPetals/drawPetalLayer reject DNA with fewer than 2 genomes", "[NativeAPI]"){
    REQUIRE_THROWS_AS(drawFlower(emptyDna(), 64, 3, 6.0f, 1.0f), std::invalid_argument);
    REQUIRE_THROWS_AS(drawPetals(oneGenomeDna(), 64, 3, 6.0f, 1.0f), std::invalid_argument);
    REQUIRE_THROWS_AS(drawPetalLayer(emptyDna(), 64, 3, 6.0f, 1.0f, 0), std::invalid_argument);
}

TEST_CASE("reproduce requires both parents to have the same genome count", "[NativeAPI]"){
    auto father = makeFlower(64, 3, 6.0f, 1.0f);
    REQUIRE_THROWS_AS(reproduce(father.dna, oneGenomeDna(), 64, 3, 6.0f, 1.0f), std::runtime_error);
}

TEST_CASE("reproduce of two valid parents yields a 2-genome child", "[NativeAPI]"){
    auto father = makeFlower(64, 3, 6.0f, 1.0f);
    auto mother = makeFlower(64, 3, 6.0f, 1.0f);
    auto child = reproduce(father.dna, mother.dna, 64, 3, 6.0f, 1.0f);
    REQUIRE(child.dna.size() == 2u);
}

TEST_CASE("mutate leaves the caller's DNA untouched (taken by value) and returns a valid Flower", "[NativeAPI]"){
    auto original = makeFlower(64, 3, 6.0f, 1.0f);
    std::stringstream before;
    original.dna.toJson().writeToStream(before, false, true);

    auto mutated = mutate(original.dna, 64, 3, 6.0f, 1.0f,
        0.2f, 0.3f, 0.2f, 0.6f, 0.35f, 0.3f, 0.4f);

    REQUIRE(mutated.dna.size() == 2u);

    std::stringstream after;
    original.dna.toJson().writeToStream(after, false, true);
    REQUIRE(before.str() == after.str());
}

TEST_CASE("mutate rejects DNA with fewer than 2 genomes", "[NativeAPI]"){
    REQUIRE_THROWS_AS(
        mutate(oneGenomeDna(), 64, 3, 6.0f, 1.0f, 0.2f, 0.3f, 0.2f, 0.6f, 0.35f, 0.3f, 0.4f),
        std::invalid_argument
    );
}

TEST_CASE("getFlowerStats forwards to fe::Stats and returns values in range", "[NativeAPI]"){
    auto flower = makeFlower(64, 3, 6.0f, 1.0f);
    JsonBox::Value v;
    v["Flower"] = flower.toJson();
    std::stringstream ss;
    v.writeToStream(ss, false, true);

    auto stats = getFlowerStats(ss.str(), 0.5f, 20, 0, 0);
    REQUIRE(stats.health >= 1);
    REQUIRE(stats.health <= 100);
}

TEST_CASE("make3DFlower rejects invalid numeric params and empty flowerId", "[NativeAPI]"){
    auto flower = makeFlower(64, 3, 6.0f, 1.0f);
    REQUIRE_THROWS_AS(make3DFlower(flower.dna, 0, 3, 6.0f, 1.0f, "id"), std::invalid_argument);
    REQUIRE_THROWS_AS(make3DFlower(flower.dna, 64, 0, 6.0f, 1.0f, "id"), std::invalid_argument);
    REQUIRE_THROWS_AS(make3DFlower(flower.dna, 64, 3, 6.0f, 1.0f, ""), std::invalid_argument);
}

TEST_CASE("make3DFlower rejects DNA with fewer than 2 genomes", "[NativeAPI]"){
    REQUIRE_THROWS_AS(make3DFlower(oneGenomeDna(), 64, 3, 6.0f, 1.0f, "id"), std::invalid_argument);
}

TEST_CASE("make3DFlower on valid input returns a well-formed glTF 2.0 JSON document", "[NativeAPI]"){
    auto flower = makeFlower(32, 2, 6.0f, 1.0f);
    auto gltf = make3DFlower(flower.dna, 32, 2, 6.0f, 1.0f, "test-flower");
    REQUIRE_FALSE(gltf.empty());

    JsonBox::Value v;
    v.loadFromString(gltf);
    REQUIRE(v["asset"]["version"].getString() == "2.0");
    REQUIRE_FALSE(v["nodes"].isNull());
    REQUIRE_FALSE(v["meshes"].isNull());
}
