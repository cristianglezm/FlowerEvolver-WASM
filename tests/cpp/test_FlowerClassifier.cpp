#include <sstream>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <fe/FlowerEvolver.hpp>
#include <fe/FlowerClassifier.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace{
    // Tests link the shared library, where fe::detail::impl_* is deliberately
    // not exported, so batches are unpacked here with the public API only.
    std::vector<std::vector<double>> extractSamples(const std::string& generationJson, const fe::Environment& env){
        JsonBox::Value v;
        v.loadFromString(generationJson);
        std::vector<std::vector<double>> samples;
        for(const auto& entry : v["Generation"].getArray()){
            JsonBox::Object wrapper;
            wrapper["Flower"] = entry;
            std::stringstream ss;
            JsonBox::Value(wrapper).writeToStream(ss, false, true);
            samples.push_back(fe::extractFlowerFeatures(ss.str(), env));
        }
        return samples;
    }

    std::string makeGenomeString(int radius = 64, int numLayers = 3, float P = 6.0f, float bias = 1.0f){
        auto flower = makeFlower(radius, numLayers, P, bias);
        JsonBox::Value v;
        v["Flower"] = flower.toJson();
        std::stringstream ss;
        v.writeToStream(ss, false, true);
        return ss.str();
    }

    // A small {"Generation": [...]} batch, `count` flowers, each with
    // slightly different P so they don't all land on the same node.
    std::string makeGenerationJson(std::size_t count){
        JsonBox::Array generation;
        for(std::size_t i = 0; i < count; ++i){
            auto flower = makeFlower(64, 3, 2.0f + static_cast<float>(i), 1.0f);
            generation.emplace_back(flower.toJson());
        }
        JsonBox::Object root;
        root["Generation"] = JsonBox::Value(generation);
        JsonBox::Value v(root);
        std::stringstream ss;
        v.writeToStream(ss, false, true);
        return ss.str();
    }

    fe::DNA oneGenomeDna(){
        fe::DNA dna;
        dna.add(EvoAI::Genome(4, 14, false, true));
        return dna;
    }
}

TEST_CASE("extractFlowerFeatures returns a vector of FlowerClassifier::inputDim values in [0,1]", "[FlowerClassifier]"){
    auto genome = makeGenomeString();
    fe::Environment env;
    auto features = fe::extractFlowerFeatures(genome, env);

    REQUIRE(features.size() == fe::FlowerClassifier::inputDim);
    for(double v : features){
        REQUIRE(v >= 0.0);
        REQUIRE(v <= 1.0);
    }
}

TEST_CASE("extractFlowerFeatures throws std::runtime_error on unparsable JSON", "[FlowerClassifier]"){
    REQUIRE_THROWS_AS(fe::extractFlowerFeatures("not json", fe::Environment{}), std::runtime_error);
}

TEST_CASE("extractFlowerFeatures throws std::invalid_argument when the DNA has fewer than 2 genomes", "[FlowerClassifier]"){
    JsonBox::Value v;
    JsonBox::Object flowerObj;
    flowerObj["dna"] = oneGenomeDna().toJson();
    v["Flower"] = JsonBox::Value(flowerObj);
    std::stringstream ss;
    v.writeToStream(ss, false, true);

    REQUIRE_THROWS_AS(fe::extractFlowerFeatures(ss.str(), fe::Environment{}), std::invalid_argument);
}

TEST_CASE("extractFlowerFeatures falls back to the petals defaults for a missing or malformed petals section", "[FlowerClassifier]"){
    JsonBox::Value v;
    v.loadFromString(makeGenomeString());
    for(const auto& bad : {JsonBox::Value(), JsonBox::Value(5), JsonBox::Value("x"), JsonBox::Value(JsonBox::Object())}){
        v["Flower"]["petals"] = bad;
        std::stringstream ss;
        v.writeToStream(ss, false, true);
        REQUIRE(fe::extractFlowerFeatures(ss.str(), fe::Environment{}).size() == fe::FlowerClassifier::inputDim);
    }
    auto flower = v["Flower"].getObject();
    flower.erase("petals");
    v["Flower"] = flower;
    std::stringstream ss;
    v.writeToStream(ss, false, true);
    REQUIRE(fe::extractFlowerFeatures(ss.str(), fe::Environment{}).size() == fe::FlowerClassifier::inputDim);
}

TEST_CASE("extractFlowerFeatures clamps an absurd petals radius and numLayers instead of failing", "[FlowerClassifier]"){
    for(auto [radius, numLayers] : {std::pair{0, 0}, std::pair{-5, -1}, std::pair{2147483647, 2147483647}}){
        auto features = fe::extractFlowerFeatures(makeGenomeString(radius, numLayers), fe::Environment{});
        REQUIRE(features.size() == fe::FlowerClassifier::inputDim);
    }
}

TEST_CASE("FlowerClassifier(width, height, environment) constructs an untrained classifier of that shape", "[FlowerClassifier]"){
    fe::Environment env(0.5f, 20, 0, 0);
    fe::FlowerClassifier classifier(4, 3, env);

    REQUIRE(classifier.som.getWidth() == 4u);
    REQUIRE(classifier.som.getHeight() == 3u);
    REQUIRE(classifier.som.getInputDim() == fe::FlowerClassifier::inputDim);
    REQUIRE(classifier.environment.temperature == 20);
}

TEST_CASE("FlowerClassifier round-trips through toJson()/the JSON constructor", "[FlowerClassifier]"){
    fe::Environment env(0.7f, 15, 100, 2);
    fe::FlowerClassifier classifier(3, 3, env);

    fe::FlowerClassifier reloaded(classifier.toJson().getObject());

    REQUIRE(reloaded.som.getWidth() == classifier.som.getWidth());
    REQUIRE(reloaded.som.getHeight() == classifier.som.getHeight());
    REQUIRE(reloaded.som.getInputDim() == classifier.som.getInputDim());
    REQUIRE(reloaded.environment.temperature == 15);
    REQUIRE(reloaded.environment.altitude == 100);
}

TEST_CASE("trainFlowerClassifier(samples, ...) trains a classifier that classifies its own training samples", "[FlowerClassifier][NativeAPI]"){
    fe::Environment env;
    std::vector<std::vector<double>> samples;
    for(int i = 0; i < 6; ++i){
        auto genome = makeGenomeString(64, 3, 2.0f + static_cast<float>(i), 1.0f);
        samples.emplace_back(fe::extractFlowerFeatures(genome, env));
    }

    fe::SOMTrainingConfig training(2, 2, 20, 0.5, 1.5);
    auto classifier = trainFlowerClassifier(samples, env, training);

    REQUIRE(classifier.som.getWidth() == 2u);
    REQUIRE(classifier.som.getHeight() == 2u);
    for(auto& sample : samples){
        auto match = classifier.som.classify(sample);
        REQUIRE(match.has_value());
        REQUIRE(match->x < 2u);
        REQUIRE(match->y < 2u);
    }
}

TEST_CASE("trainFlowerClassifier(generationsJson, ...) trains from a whole Generation batch", "[FlowerClassifier][NativeAPI]"){
    auto generationJson = makeGenerationJson(5);
    fe::Environment env;
    fe::SOMTrainingConfig training(2, 2, 10, 0.5, 1.5);

    auto classifier = trainFlowerClassifier(generationJson, env, training);

    REQUIRE(classifier.som.getWidth() == 2u);
    REQUIRE(classifier.som.getHeight() == 2u);
}

TEST_CASE("trainFlowerClassifier(generationsJson, ...) skips unusable flowers instead of aborting", "[FlowerClassifier][NativeAPI]"){
    JsonBox::Array generation;
    // one good flower
    generation.emplace_back(makeFlower(64, 3, 6.0f, 1.0f).toJson());
    // one bad entry: dna with only 1 genome (petals included so the only
    // thing wrong with it is the DNA, keeping the test's intent unambiguous)
    JsonBox::Object badFlower;
    badFlower["dna"] = oneGenomeDna().toJson();
    badFlower["petals"] = makeFlower(64, 3, 6.0f, 1.0f).petals.toJson();
    generation.emplace_back(JsonBox::Value(badFlower));

    JsonBox::Object root;
    root["Generation"] = JsonBox::Value(generation);
    JsonBox::Value v(root);
    std::stringstream ss;
    v.writeToStream(ss, false, true);

    fe::Environment env;
    fe::SOMTrainingConfig training(2, 2, 5, 0.5, 1.5);
    REQUIRE_NOTHROW(trainFlowerClassifier(ss.str(), env, training));
}

TEST_CASE("trainFlowerClassifier(generationsJson, ...) throws std::runtime_error on unparsable JSON", "[FlowerClassifier][NativeAPI]"){
    fe::Environment env;
    fe::SOMTrainingConfig training;
    REQUIRE_THROWS_AS(trainFlowerClassifier(std::string("not json"), env, training), std::runtime_error);
}

TEST_CASE("FlowerClassifier::classify(genome, ...) finds a BMU for a genome it was trained on", "[FlowerClassifier]"){
    auto generationJson = makeGenerationJson(6);
    fe::Environment env;
    fe::SOMTrainingConfig training(2, 2, 20, 0.5, 1.5);
    auto classifier = trainFlowerClassifier(generationJson, env, training);

    auto genome = makeGenomeString(64, 3, 3.0f, 1.0f);
    auto match = classifier.classify(genome, env);

    REQUIRE(match.has_value());
    REQUIRE(match->x < 2u);
    REQUIRE(match->y < 2u);
    REQUIRE(std::isfinite(match->distance));
    REQUIRE(match->distance >= 0.0);
}

TEST_CASE("FlowerClassifier::classify(genome, ...) returns nullopt instead of throwing on a bad genome", "[FlowerClassifier]"){
    fe::FlowerClassifier classifier(2, 2, fe::Environment{});
    REQUIRE_FALSE(classifier.classify(std::string("not json"), fe::Environment{}).has_value());

    JsonBox::Value v;
    JsonBox::Object flowerObj;
    flowerObj["dna"] = oneGenomeDna().toJson();
    v["Flower"] = JsonBox::Value(flowerObj);
    std::stringstream ss;
    v.writeToStream(ss, false, true);
    REQUIRE_FALSE(classifier.classify(ss.str(), fe::Environment{}).has_value());
}

TEST_CASE("FlowerClassifier::classify(fe::Flower, ...) agrees with the genome-string overload", "[FlowerClassifier]"){
    auto generationJson = makeGenerationJson(6);
    fe::Environment env;
    fe::SOMTrainingConfig training(2, 2, 20, 0.5, 1.5);
    auto classifier = trainFlowerClassifier(generationJson, env, training);

    auto flower = makeFlower(64, 3, 3.0f, 1.0f);
    auto viaFlower = classifier.classify(flower, env);

    JsonBox::Value v;
    v["Flower"] = flower.toJson();
    std::stringstream ss;
    v.writeToStream(ss, false, true);
    auto viaGenome = classifier.classify(ss.str(), env);

    REQUIRE(viaFlower.has_value());
    REQUIRE(viaGenome.has_value());
    // the same flower: the same species. The distances only agree to the noise of the extraction
    REQUIRE(viaFlower->x == viaGenome->x);
    REQUIRE(viaFlower->y == viaGenome->y);
    REQUIRE(viaFlower->distance == Catch::Approx(viaGenome->distance).margin(0.5));
}

TEST_CASE("SOMTrainingConfig rejects zero sizes/epochs and non-positive rates", "[FlowerClassifier]"){
    REQUIRE_THROWS_AS(fe::SOMTrainingConfig(0, 5, 100, 0.5, 2.5), std::invalid_argument);
    REQUIRE_THROWS_AS(fe::SOMTrainingConfig(5, 0, 100, 0.5, 2.5), std::invalid_argument);
    REQUIRE_THROWS_AS(fe::SOMTrainingConfig(5, 5, 0, 0.5, 2.5), std::invalid_argument);
    REQUIRE_THROWS_AS(fe::SOMTrainingConfig(5, 5, 100, 0.0, 2.5), std::invalid_argument);
    REQUIRE_THROWS_AS(fe::SOMTrainingConfig(5, 5, 100, 0.5, -1.0), std::invalid_argument);
    REQUIRE_NOTHROW(fe::SOMTrainingConfig(5, 5, 100, 0.5, 2.5));
}

TEST_CASE("SOMTrainingConfig(JsonBox::Object) rejects a negative int instead of wrapping to SIZE_MAX", "[FlowerClassifier]"){
    JsonBox::Object negative;
    negative["width"] = -1;
    REQUIRE_THROWS_AS(fe::SOMTrainingConfig(negative), std::invalid_argument);

    JsonBox::Object partial;
    partial["width"] = 8;
    fe::SOMTrainingConfig cfg(partial); // missing fields still default, not throw
    REQUIRE(cfg.width == 8);
    REQUIRE(cfg.height == 5);
}

TEST_CASE("Environment rejects a non-finite humidity but allows unusual finite ones", "[FlowerClassifier]"){
    REQUIRE_THROWS_AS(fe::Environment(std::numeric_limits<float>::quiet_NaN(), 20, 0, 0), std::invalid_argument);
    REQUIRE_THROWS_AS(fe::Environment(std::numeric_limits<float>::infinity(), 20, 0, 0), std::invalid_argument);

    JsonBox::Object overflow;
    overflow["humidity"] = 1e300; // valid JSON, overflows float to Inf
    REQUIRE_THROWS_AS(fe::Environment(overflow), std::invalid_argument);

    REQUIRE_NOTHROW(fe::Environment(5.0f, 20, 0, 0));
}

TEST_CASE("FlowerClassifier(JsonBox::Object) rejects a classifier with the wrong input dimension", "[FlowerClassifier]"){
    EvoAI::SelfOrganizingMap<> wrongDim(3, 3, fe::FlowerClassifier::inputDim - 1);
    JsonBox::Object o;
    o["som"] = wrongDim.toJson();
    o["environment"] = fe::Environment().toJson();
    REQUIRE_THROWS_AS(fe::FlowerClassifier(o), std::invalid_argument);
}

TEST_CASE("FlowerClassifier::train rejects a non-positive or non-finite baseLR/baseRadius", "[FlowerClassifier]"){
    fe::FlowerClassifier classifier(2, 2, fe::Environment{});
    auto before = classifier.toJson();
    std::vector<std::vector<double>> samples{std::vector<double>(fe::FlowerClassifier::inputDim, 0.5)};
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();

    for(double bad : {0.0, -1.0, nan, inf}){
        REQUIRE_THROWS_AS(classifier.train(samples, 5, bad, 1.0), std::invalid_argument);
        REQUIRE_THROWS_AS(classifier.train(samples, 5, 0.5, bad), std::invalid_argument);
    }
    // rejected before touching the weights
    std::stringstream a, b;
    before.writeToStream(a, false, true);
    classifier.toJson().writeToStream(b, false, true);
    REQUIRE(a.str() == b.str());
    REQUIRE_NOTHROW(classifier.train(samples, 5, 0.5, 1.0));
}

TEST_CASE("classifyFlowers returns one entry per flower in order, null for an unusable one", "[FlowerClassifier]"){
    fe::Environment env;
    auto classifier = trainFlowerClassifier(makeGenerationJson(6), env, fe::SOMTrainingConfig(3, 2, 5, 0.5, 1.5));

    JsonBox::Value parsed;
    parsed.loadFromString(makeGenerationJson(3));
    JsonBox::Array generation = parsed["Generation"].getArray();
    generation.insert(generation.begin() + 1, JsonBox::Value(JsonBox::Object{})); // unusable, in the middle
    JsonBox::Value v;
    v["Generation"] = JsonBox::Value(generation);
    std::stringstream ss;
    v.writeToStream(ss, false, true);

    auto matches = classifyFlowers(classifier, ss.str(), env);
    REQUIRE(matches.size() == 4u);
    REQUIRE_FALSE(matches[1].has_value());
    for(std::size_t i : {0u, 2u, 3u}){
        REQUIRE(matches[i].has_value());
        REQUIRE(matches[i]->x < 3u);
        REQUIRE(matches[i]->y < 2u);
    }
    REQUIRE(classifyFlowers(classifier, "{\"Generation\":[]}", env).empty());
    REQUIRE_THROWS_AS(classifyFlowers(classifier, "not json", env), std::runtime_error);
    fe::Environment bad;
    bad.humidity = std::numeric_limits<float>::infinity();
    REQUIRE_THROWS_AS(classifyFlowers(classifier, ss.str(), bad), std::invalid_argument);
}

TEST_CASE("fields changed after construction are re-validated where they are used", "[FlowerClassifier]"){
    std::vector<std::vector<double>> samples{std::vector<double>(fe::FlowerClassifier::inputDim, 0.5)};
    const float nan = std::numeric_limits<float>::quiet_NaN();

    fe::SOMTrainingConfig cfg(2, 2, 5, 0.5, 1.5);
    fe::Environment env;
    REQUIRE_NOTHROW(cfg.validate());
    REQUIRE_NOTHROW(env.validate());

    SECTION("SOMTrainingConfig"){
        cfg.baseLR = 0.0;
        REQUIRE_THROWS_AS(cfg.validate(), std::invalid_argument);
        REQUIRE_THROWS_AS(trainFlowerClassifier(samples, env, cfg), std::invalid_argument);
        REQUIRE_THROWS_AS(trainFlowerClassifier(makeGenerationJson(2), env, cfg), std::invalid_argument);
        cfg.baseLR = 0.5;
        cfg.width = 0;
        REQUIRE_THROWS_AS(trainFlowerClassifier(samples, env, cfg), std::invalid_argument);
        cfg.width = 2;
        cfg.epochs = 0;
        REQUIRE_THROWS_AS(cfg.validate(), std::invalid_argument);
    }
    SECTION("Environment"){
        env.humidity = nan;
        REQUIRE_THROWS_AS(env.validate(), std::invalid_argument);
        REQUIRE_THROWS_AS(fe::extractFlowerFeatures(makeGenomeString(), env), std::invalid_argument);
        REQUIRE_THROWS_AS(trainFlowerClassifier(samples, env, cfg), std::invalid_argument);
        // a bad environment is not "every flower was unusable"
        REQUIRE_THROWS_AS(trainFlowerClassifier(makeGenerationJson(2), env, cfg), std::invalid_argument);
        fe::FlowerClassifier classifier(2, 2, fe::Environment{});
        REQUIRE_THROWS_AS(classifier.classify(makeGenomeString(), env), std::invalid_argument);
        REQUIRE_THROWS_AS(classifyFlowers(classifier, makeGenerationJson(2), env), std::invalid_argument);
    }
}

TEST_CASE("train() split over several calls follows the same decay as one call (single sample, order-free)", "[FlowerClassifier]"){
    fe::Environment env;
    // one sample: train() reshuffles the sample order inside every call, so with
    // several samples the chunks would differ by order, not by decay
    auto samples = extractSamples(makeGenerationJson(1), env);
    REQUIRE(samples.size() == 1u);
    fe::FlowerClassifier single(3, 2, env);
    // identical starting weights: round trip through JSON
    fe::FlowerClassifier chunked(JsonBox::Object(single.toJson().getObject()));
    fe::FlowerClassifier restarted(JsonBox::Object(single.toJson().getObject()));

    auto maxDiff = [](const fe::FlowerClassifier& a, const fe::FlowerClassifier& b){
        double worst = 0.0;
        for(std::size_t y = 0; y < 2; ++y){
            for(std::size_t x = 0; x < 3; ++x){
                auto pa = a.som.getPrototype(x, y);
                auto pb = b.som.getPrototype(x, y);
                for(std::size_t k = 0; k < pa.size(); ++k){
                    worst = std::max(worst, std::abs(pa[k] - pb[k]));
                }
            }
        }
        return worst;
    };
    REQUIRE(maxDiff(single, chunked) == 0.0);

    EvoAI::randomGen().setSeed(7);
    single.train(samples, 20, 0.5, 1.5);
    EvoAI::randomGen().setSeed(7);
    chunked.train(samples, 5, 0.5, 1.5, 0, 20);
    chunked.train(samples, 10, 0.5, 1.5, 5, 20);
    chunked.train(samples, 5, 0.5, 1.5, 15, 20);
    EvoAI::randomGen().setSeed(7);
    restarted.train(samples, 10, 0.5, 1.5);
    restarted.train(samples, 10, 0.5, 1.5);

    REQUIRE(maxDiff(single, chunked) < 1e-9);
    // without offsets the second half restarts the decay, which is not the same run
    REQUIRE(maxDiff(single, restarted) > 1e-3);

    REQUIRE_THROWS_AS(single.train(samples, 10, 0.5, 1.5, 15, 20), std::invalid_argument); // 25 > 20
    REQUIRE_THROWS_AS(single.train(samples, 10, 0.5, 1.5, std::numeric_limits<std::size_t>::max(), 0), std::invalid_argument);
}

TEST_CASE("FlowerClassifier::evaluate agrees with classify()/getPrototype() computed by hand", "[FlowerClassifier]"){
    fe::Environment env;
    auto samples = extractSamples(makeGenerationJson(8), env);
    fe::FlowerClassifier classifier(3, 2, env);
    classifier.train(samples, 5, 0.5, 1.5);

    auto withBad = samples;
    withBad.push_back(std::vector<double>(3, 0.5)); // wrong size
    auto e = classifier.evaluate(withBad);

    std::vector<std::size_t> counts(6, 0u);
    double sum = 0.0;
    for(const auto& sample : samples){
        auto bmu = *classifier.som.classify(sample);
        auto proto = classifier.som.getPrototype(bmu.x, bmu.y);
        double sq = 0.0;
        for(std::size_t k = 0; k < sample.size(); ++k){ sq += (sample[k] - proto[k]) * (sample[k] - proto[k]); }
        sum += std::sqrt(sq);
        ++counts[bmu.y * 3 + bmu.x];
    }
    REQUIRE(e.samplesUsed == samples.size());
    REQUIRE(e.samplesSkipped == 1u);
    REQUIRE(e.totalNodes == 6u);
    REQUIRE(e.nodeCounts == counts);
    REQUIRE(e.meanQuantizationError == Catch::Approx(sum / static_cast<double>(samples.size())));
    REQUIRE(e.usedNodes == static_cast<std::size_t>(std::count_if(counts.begin(), counts.end(), [](auto c){ return c > 0u; })));
    REQUIRE(e.busiestNodeCount == *std::max_element(counts.begin(), counts.end()));
    REQUIRE(e.fractionUsed == Catch::Approx(static_cast<double>(e.usedNodes) / 6.0));
    REQUIRE(e.samplesPerNodeMean == Catch::Approx(static_cast<double>(samples.size()) / 6.0));

    // the free function works on a bare EvoAI SOM, FlowerClassifier::evaluate() just delegates to it
    auto direct = fe::evaluateSOM(classifier.som, withBad);
    REQUIRE(direct.nodeCounts == e.nodeCounts);
    REQUIRE(direct.meanQuantizationError == e.meanQuantizationError);
    REQUIRE_THROWS_AS(fe::evaluateSOM(classifier.som, {}), std::invalid_argument);

    REQUIRE_THROWS_AS(classifier.evaluate({}), std::invalid_argument);
    REQUIRE_THROWS_AS(classifier.evaluate({std::vector<double>(3, 0.5)}), std::invalid_argument);

    auto j = e.toJson();
    REQUIRE(j["nodeCounts"].getArray().size() == 6u);
    REQUIRE(j["usedNodes"].getInteger() == static_cast<int>(e.usedNodes));
}

TEST_CASE("trainFlowerClassifier(samples, ...) throws when no sample is usable", "[FlowerClassifier][NativeAPI]"){
    fe::SOMTrainingConfig cfg(3, 3, 5, 0.5, 1.5);
    REQUIRE_THROWS_AS(trainFlowerClassifier(std::vector<std::vector<double>>{}, fe::Environment(), cfg), std::invalid_argument);
    std::vector<std::vector<double>> wrongSize{{1.0, 2.0}, {3.0}};
    REQUIRE_THROWS_AS(trainFlowerClassifier(wrongSize, fe::Environment(), cfg), std::invalid_argument);
}

TEST_CASE("FlowerClassifier::train trains in place, reports skipped samples, and epochs=0 changes nothing", "[FlowerClassifier]"){
    auto asString = [](const fe::FlowerClassifier& c){
        std::stringstream ss;
        c.toJson().writeToStream(ss, false, true);
        return ss.str();
    };
    fe::FlowerClassifier classifier(3, 3, fe::Environment());
    std::vector<std::vector<double>> samples{
        std::vector<double>(fe::FlowerClassifier::inputDim, 0.25),
        std::vector<double>(fe::FlowerClassifier::inputDim, 0.75),
        {1.0} // wrong size
    };
    auto untrained = asString(classifier);

    REQUIRE(classifier.train(samples, 0, 0.5, 1.5).skipped.size() <= 1);
    REQUIRE(asString(classifier) == untrained);

    REQUIRE(classifier.train(samples, 10, 0.5, 1.5).skipped.size() == 1);
    auto trained = asString(classifier);
    REQUIRE(trained != untrained);

    std::vector<std::vector<double>> allBad{{1.0}, {2.0, 3.0}};
    REQUIRE_THROWS_AS(classifier.train(allBad, 10, 0.5, 1.5), std::invalid_argument);
    REQUIRE(asString(classifier) == trained);
}

TEST_CASE("classifyFeatures gives the node and the distance of a feature vector", "[FlowerClassifier]"){
    fe::Environment env;
    fe::FlowerClassifier classifier(3, 2, env);
    // a prototype is its own best match, at distance 0
    auto prototype = classifier.som.getPrototype(2u, 1u);
    auto atPrototype = classifier.classifyFeatures(prototype);
    REQUIRE(atPrototype.has_value());
    REQUIRE(atPrototype->x == 2u);
    REQUIRE(atPrototype->y == 1u);
    REQUIRE(atPrototype->distance == 0.0);
    // moving away from it by 0.1 in one value is a distance of 0.1 and, nearby, the same node
    auto moved = prototype;
    moved[5] += 0.1;
    auto nearby = classifier.classifyFeatures(moved);
    REQUIRE(nearby.has_value());
    REQUIRE(nearby->distance <= 0.1 + 1e-12);
    // the distance is the euclidean one to the prototype of the node it names
    auto proto = classifier.som.getPrototype(nearby->x, nearby->y);
    double sq = 0.0;
    for(std::size_t k = 0; k < moved.size(); ++k){ sq += (moved[k] - proto[k]) * (moved[k] - proto[k]); }
    REQUIRE(nearby->distance == Catch::Approx(std::sqrt(sq)).margin(1e-12));
    // wrong size and non-finite vectors are not classified
    REQUIRE_FALSE(classifier.classifyFeatures(std::vector<double>(3, 0.5)).has_value());
    auto bad = prototype;
    bad[0] = std::numeric_limits<double>::quiet_NaN();
    REQUIRE_FALSE(classifier.classifyFeatures(bad).has_value());
}

TEST_CASE("train() reports every epoch, the skipped samples and the wins per node", "[FlowerClassifier]"){
    fe::FlowerClassifier classifier(3, 2, fe::Environment());
    std::vector<std::vector<double>> samples;
    for(int i = 0; i < 6; ++i){
        samples.emplace_back(fe::FlowerClassifier::inputDim, 0.1 * (i + 1));
    }
    samples.insert(samples.begin() + 3, std::vector<double>(3, 0.5)); // wrong size, index 3

    auto report = classifier.train(samples, 5, 0.5, 1.5);
    REQUIRE(report.skipped == std::vector<std::size_t>{3u});
    REQUIRE(report.epochs.size() == 5u);
    for(std::size_t i = 0; i < report.epochs.size(); ++i){
        REQUIRE(report.epochs[i].epoch == i);
        REQUIRE(std::isfinite(report.epochs[i].meanDistance));
        REQUIRE(report.epochs[i].meanDistance >= 0.0);
        if(i > 0){
            REQUIRE(report.epochs[i].learningRate < report.epochs[i - 1].learningRate);
            REQUIRE(report.epochs[i].radius < report.epochs[i - 1].radius);
        }
    }
    // the decay ends at its floors (0.01 and 0.5) in the last epoch of the whole decay
    REQUIRE(report.epochs.back().learningRate == Catch::Approx(0.01));
    REQUIRE(report.epochs.back().radius == Catch::Approx(0.5));
    // the six usable samples are spread over the six nodes
    REQUIRE(report.nodeCounts.size() == 6u);
    std::size_t won = 0;
    for(auto count : report.nodeCounts){ won += count; }
    REQUIRE(won == 6u);
}

TEST_CASE("train() continues the numbering of the epochs with epochOffset", "[FlowerClassifier]"){
    fe::FlowerClassifier classifier(2, 2, fe::Environment());
    std::vector<std::vector<double>> samples{
        std::vector<double>(fe::FlowerClassifier::inputDim, 0.25),
        std::vector<double>(fe::FlowerClassifier::inputDim, 0.75)};
    auto first = classifier.train(samples, 3, 0.5, 1.5, 0, 5);
    auto second = classifier.train(samples, 2, 0.5, 1.5, 3, 5);
    REQUIRE(first.epochs.back().epoch == 2u);
    REQUIRE(second.epochs.front().epoch == 3u);
    REQUIRE(second.epochs.back().epoch == 4u);
    // the same decay as one call of five
    fe::FlowerClassifier whole(2, 2, fe::Environment());
    auto all = whole.train(samples, 5, 0.5, 1.5);
    REQUIRE(second.epochs.back().learningRate == Catch::Approx(all.epochs.back().learningRate));
    REQUIRE(second.epochs.front().learningRate == Catch::Approx(all.epochs[3].learningRate));
}

TEST_CASE("train() calls onEpoch after every epoch and stops when it returns false", "[FlowerClassifier]"){
    fe::FlowerClassifier classifier(2, 2, fe::Environment());
    std::vector<std::vector<double>> samples{
        std::vector<double>(fe::FlowerClassifier::inputDim, 0.25),
        std::vector<double>(fe::FlowerClassifier::inputDim, 0.75)};
    std::vector<std::size_t> seen;
    auto report = classifier.train(samples, 20, 0.5, 1.5, 0, 0, [&seen](const fe::SOMEpoch& epoch){
        seen.push_back(epoch.epoch);
        return epoch.epoch < 3u;
    });
    REQUIRE(seen == std::vector<std::size_t>{0u, 1u, 2u, 3u});
    REQUIRE(report.epochs.size() == 4u); // the epoch that asked to stop is in the report

    // an exception from the callback reaches the caller
    REQUIRE_THROWS_AS(classifier.train(samples, 5, 0.5, 1.5, 0, 0, [](const fe::SOMEpoch&) -> bool{
        throw std::runtime_error("stop");
    }), std::runtime_error);
}
