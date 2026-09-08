#include <catch2/catch_test_macros.hpp>

#include <fe/FlowerEvolver.hpp>
#include <fe/Stats.hpp>

namespace {
    std::string makeGenomeString(){
        auto flower = makeFlower(64, 3, 6.0f, 1.0f);
        JsonBox::Value v;
        v["Flower"] = flower.toJson();
        std::stringstream ss;
        v.writeToStream(ss, false, true);
        return ss.str();
    }
}

TEST_CASE("Stats() throws on unparsable genome JSON", "[Stats]"){
    REQUIRE_THROWS_AS(fe::Stats("not json", 0.5f, 20, 0, 0), std::runtime_error);
}

TEST_CASE("Stats() throws when the genome has no dna", "[Stats]"){
    REQUIRE_THROWS_AS(fe::Stats(R"({"Flower":{}})", 0.5f, 20, 0, 0), std::invalid_argument);
}

TEST_CASE("Stats() throws when the DNA has fewer than 2 genomes", "[Stats]"){
    fe::DNA dna;
    dna.add(EvoAI::Genome(4, 14, false, true)); // stats genome only
    JsonBox::Value v;
    JsonBox::Object flowerObj;
    flowerObj["dna"] = dna.toJson();
    v["Flower"] = JsonBox::Value(flowerObj);
    std::stringstream ss;
    v.writeToStream(ss, false, true);

    REQUIRE_THROWS_AS(fe::Stats(ss.str(), 0.5f, 20, 0, 0), std::invalid_argument);
}

TEST_CASE("Stats() throws when dna[0] has 2 entries but no actual node chromosomes", "[Stats]"){
    REQUIRE_THROWS_AS(
        fe::Stats(R"({"Flower":{"dna":{"genomes":[{},{}]}}})", 0.5f, 20, 0, 0),
        std::invalid_argument
    );
}

TEST_CASE("Stats() throws when dna[0] has real outputs but fewer than the 14 it reads", "[Stats]"){
    fe::DNA dna;
    dna.add(EvoAI::Genome(4, 13, false, true));
    dna.add(EvoAI::Genome(4, 4, false, true));
    JsonBox::Value v;
    JsonBox::Object flowerObj;
    flowerObj["dna"] = dna.toJson();
    v["Flower"] = JsonBox::Value(flowerObj);
    std::stringstream ss;
    v.writeToStream(ss, false, true);

    REQUIRE_THROWS_AS(fe::Stats(ss.str(), 0.5f, 20, 0, 0), std::invalid_argument);
}

TEST_CASE("Stats() derived fields stay within their documented/clamped ranges", "[Stats]"){
    auto genome = makeGenomeString();
    fe::Stats stats(genome, 0.5f, 20, 0, 0);

    REQUIRE(stats.health >= 1);
    REQUIRE(stats.health <= 100);
    REQUIRE(stats.stamina >= 1);
    REQUIRE(stats.stamina <= 100);
    REQUIRE(stats.minTemperature >= 0);
    REQUIRE(stats.maxTemperature <= 50);
    REQUIRE(stats.minTemperature <= stats.maxTemperature);
    REQUIRE(stats.maturationPeriod >= 1);
    REQUIRE(stats.maturationPeriod <= 100);
    REQUIRE(stats.toxicityRate >= -100.0);
    REQUIRE(stats.toxicityRate <= 100.0);
    REQUIRE((stats.sex == fe::Stats::Male || stats.sex == fe::Stats::Female || stats.sex == fe::Stats::Both));

    for(double effect : {stats.effects.vitality, stats.effects.agility, stats.effects.intelligence,
                          stats.effects.strength, stats.effects.luck}){
        REQUIRE(effect >= -100.0);
        REQUIRE(effect <= 100.0);
    }
}

TEST_CASE("Stats() is deterministic for the same genome, environment inputs, and RNG state", "[Stats]"){
    auto genome = makeGenomeString();
    EvoAI::randomGen().setSeed(12345);
    fe::Stats a(genome, 0.5f, 20, 0, 0);
    EvoAI::randomGen().setSeed(12345);
    fe::Stats b(genome, 0.5f, 20, 0, 0);

    REQUIRE(a.health == b.health);
    REQUIRE(a.stamina == b.stamina);
    REQUIRE(a.sex == b.sex);
    REQUIRE(a.toxicityRate == b.toxicityRate);
}

TEST_CASE("toJson() emits the sex enum as its string form", "[Stats]"){
    auto genome = makeGenomeString();
    fe::Stats stats(genome, 0.5f, 20, 0, 0);
    auto json = stats.toJson();
    auto sexStr = json["sex"].getString();
    REQUIRE((sexStr == "male" || sexStr == "female" || sexStr == "both"));

    auto effects = json["effects"].getObject();
    REQUIRE(effects.count("vitality") == 1u);
    REQUIRE(effects.count("agility") == 1u);
    REQUIRE(effects.count("intelligence") == 1u);
    REQUIRE(effects.count("strength") == 1u);
    REQUIRE(effects.count("luck") == 1u);
}
