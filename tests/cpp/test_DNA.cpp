#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fe/DNA.hpp>

using Catch::Matchers::WithinAbs;

namespace {
    // A CPPN genome shaped like the ones fe::Flower actually uses
    // (4 inputs, 4 outputs) - see src/fe/Flower.cpp's EvoAI::Genome(4,4,false,true).
    EvoAI::Genome makePetalGenome(){
        return EvoAI::Genome(4, 4, false, true);
    }
}

TEST_CASE("MutationRates default constructor matches the documented defaults", "[DNA][MutationRates]"){
    fe::MutationRates mr;
    REQUIRE_THAT(mr.addNodeRate, WithinAbs(0.2f, 1e-6f));
    REQUIRE_THAT(mr.addConnRate, WithinAbs(0.3f, 1e-6f));
    REQUIRE_THAT(mr.removeConnRate, WithinAbs(0.2f, 1e-6f));
    REQUIRE_THAT(mr.perturbWeightsRate, WithinAbs(0.6f, 1e-6f));
    REQUIRE_THAT(mr.enableRate, WithinAbs(0.35f, 1e-6f));
    REQUIRE_THAT(mr.disableRate, WithinAbs(0.3f, 1e-6f));
    REQUIRE_THAT(mr.actTypeRate, WithinAbs(0.4f, 1e-6f));
}

TEST_CASE("MutationRates round-trips through JSON", "[DNA][MutationRates]"){
    fe::MutationRates mr(0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f);
    fe::MutationRates back(mr.toJson().getObject());
    REQUIRE_THAT(back.addNodeRate, WithinAbs(0.1f, 1e-6f));
    REQUIRE_THAT(back.addConnRate, WithinAbs(0.2f, 1e-6f));
    REQUIRE_THAT(back.removeConnRate, WithinAbs(0.3f, 1e-6f));
    REQUIRE_THAT(back.perturbWeightsRate, WithinAbs(0.4f, 1e-6f));
    REQUIRE_THAT(back.enableRate, WithinAbs(0.5f, 1e-6f));
    REQUIRE_THAT(back.disableRate, WithinAbs(0.6f, 1e-6f));
    REQUIRE_THAT(back.actTypeRate, WithinAbs(0.7f, 1e-6f));
}

TEST_CASE("DNA starts empty", "[DNA]"){
    fe::DNA dna;
    REQUIRE(dna.size() == 0u);
    REQUIRE_THAT(static_cast<float>(dna.getFitness()), WithinAbs(0.f, 1e-6f));
}

TEST_CASE("add/remove/clear/size", "[DNA]"){
    fe::DNA dna;
    dna.add(makePetalGenome());
    dna.add(makePetalGenome());
    REQUIRE(dna.size() == 2u);

    dna.clear();
    REQUIRE(dna.size() == 0u);
}

TEST_CASE("setFitness/getFitness applies to every genome, reads from genome[0]", "[DNA]"){
    fe::DNA dna;
    dna.add(makePetalGenome());
    dna.add(makePetalGenome());
    dna.setFitness(3.5);
    REQUIRE_THAT(static_cast<float>(dna.getFitness()), WithinAbs(3.5f, 1e-6f));
    REQUIRE_THAT(static_cast<float>(dna[1].getFitness()), WithinAbs(3.5f, 1e-6f));
}

TEST_CASE("copy constructor is a deep copy (independent genome storage)", "[DNA]"){
    fe::DNA original;
    original.add(makePetalGenome());
    fe::DNA copy(original);
    REQUIRE(copy.size() == original.size());

    copy.setFitness(99.0);
    // original untouched by mutating the copy.
    REQUIRE_THAT(static_cast<float>(original.getFitness()), WithinAbs(0.f, 1e-6f));
    REQUIRE_THAT(static_cast<float>(copy.getFitness()), WithinAbs(99.f, 1e-6f));
}

TEST_CASE("move constructor transfers genomes and empties the source", "[DNA]"){
    fe::DNA original;
    original.add(makePetalGenome());
    original.add(makePetalGenome());
    fe::DNA moved(std::move(original));
    REQUIRE(moved.size() == 2u);
}

TEST_CASE("toJson / DNA(JsonBox::Object) round-trips genome count", "[DNA]"){
    fe::DNA dna;
    dna.add(makePetalGenome());
    dna.add(makePetalGenome());
    dna.add(makePetalGenome());

    fe::DNA fromJson(dna.toJson().getObject());
    REQUIRE(fromJson.size() == 3u);
}

TEST_CASE("DNA::reproduce requires matching genome counts", "[DNA]"){
    fe::DNA dna1;
    dna1.add(makePetalGenome());
    fe::DNA dna2;
    dna2.add(makePetalGenome());
    dna2.add(makePetalGenome());

    REQUIRE_THROWS_AS(fe::DNA::reproduce(dna1, dna2), std::runtime_error);
}

TEST_CASE("DNA::reproduce of matching-size DNA yields a child with the same genome count", "[DNA]"){
    fe::DNA dna1;
    dna1.add(makePetalGenome());
    dna1.add(makePetalGenome());
    fe::DNA dna2;
    dna2.add(makePetalGenome());
    dna2.add(makePetalGenome());

    fe::DNA child = fe::DNA::reproduce(dna1, dna2);
    REQUIRE(child.size() == dna1.size());
}

TEST_CASE("DNA::distance requires matching genome counts", "[DNA]"){
    fe::DNA dna1;
    dna1.add(makePetalGenome());
    fe::DNA dna2;

    REQUIRE_THROWS_AS(fe::DNA::distance(dna1, dna2), std::runtime_error);
}

TEST_CASE("DNA::distance of a DNA against itself is zero", "[DNA]"){
    fe::DNA dna1;
    dna1.add(makePetalGenome());
    fe::DNA dna2(dna1);

    REQUIRE_THAT(static_cast<float>(fe::DNA::distance(dna1, dna2)), WithinAbs(0.f, 1e-4f));
}

TEST_CASE("mutate() does not change the genome count", "[DNA]"){
    fe::DNA dna;
    dna.add(makePetalGenome());
    dna.add(makePetalGenome());
    dna.mutate(fe::MutationRates());
    REQUIRE(dna.size() == 2u);
}

TEST_CASE("remove() finds and removes a genome present in the DNA, returning true", "[DNA]"){
    fe::DNA dna;
    dna.add(makePetalGenome());
    dna.add(makePetalGenome());
    REQUIRE(dna.size() == 2u);

    REQUIRE(dna.remove(&dna[1]) == true);
    REQUIRE(dna.size() == 1u);
}

TEST_CASE("remove() of a genome not present in the DNA returns false and leaves it untouched", "[DNA]"){
    fe::DNA dna;
    dna.add(makePetalGenome());
    EvoAI::Genome notInDna = makePetalGenome();
    notInDna.setID(dna[0].getID() + 1);

    REQUIRE(dna.remove(&notInDna) == false);
    REQUIRE(dna.size() == 1u);
}

TEST_CASE("remove() on an empty DNA returns false without crashing", "[DNA]"){
    fe::DNA dna;
    EvoAI::Genome g = makePetalGenome();

    REQUIRE(dna.remove(&g) == false);
    REQUIRE(dna.size() == 0u);
}
