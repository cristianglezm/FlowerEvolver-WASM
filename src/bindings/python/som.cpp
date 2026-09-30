#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <fe/Environment.hpp>
#include <fe/FlowerClassifier.hpp>
#include <fe/FlowerEvolver.hpp>
#include <fe/Resources.hpp>
#include <fe/SOMTrainingConfig.hpp>
#include <fe/SOMTrainingReport.hpp>

#include "json_helpers.hpp"

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

void bind_som(nb::module_& m){
    nb::class_<fe::Environment>(m, "Environment",
        "The environment a flower's Stats are computed under (humidity, "
        "temperature, altitude, terrain type). The game varies this per "
        "flower on purpose (weather affects a flower's stats), so a "
        "FlowerClassifier's own recorded environment is documentation "
        "only -- classify() never checks a genome's environment against it.")
        .def(nb::init<>(), "Defaults: humidity=0.5, temperature=20, altitude=0, terrain_type=0.")
        .def(nb::init<float, int, int, int>(),
             "humidity"_a, "temperature"_a, "altitude"_a, "terrain_type"_a)
        .def_rw("humidity", &fe::Environment::humidity)
        .def_rw("temperature", &fe::Environment::temperature)
        .def_rw("altitude", &fe::Environment::altitude)
        .def_rw("terrain_type", &fe::Environment::terrainType)
        .def("validate", &fe::Environment::validate, "Raises ValueError if humidity is NaN/Inf. Called again wherever an Environment is used.")
        .def("to_json", [](const fe::Environment& e){ return jsonToString(e.toJson()); },
             "Compact JSON: {\"humidity\":..., \"temperature\":..., \"altitude\":..., \"terrainType\":...}")
        .def_static("from_json", [](const std::string& json){
                return fe::Environment(parseJsonObject(json, "Environment"));
            }, "json"_a)
        .def("__repr__", [](const fe::Environment& e){
            return "Environment(humidity=" + std::to_string(e.humidity) +
                ", temperature=" + std::to_string(e.temperature) +
                ", altitude=" + std::to_string(e.altitude) +
                ", terrain_type=" + std::to_string(e.terrainType) + ")";
        });

    nb::class_<fe::SOMTrainingConfig>(m, "SOMTrainingConfig",
        "SOM grid size + training hyperparameters for train_flower_classifier(). "
        "Defaults are untuned placeholders -- see tools/ for evaluating real "
        "grid sizes/hyperparameters against actual training data before "
        "picking final ones.")
        .def(nb::init<>(), "Defaults: width=5, height=5, epochs=100, base_lr=0.5, base_radius=2.5.")
        .def(nb::init<std::size_t, std::size_t, std::size_t, double, double>(),
             "width"_a, "height"_a, "epochs"_a, "base_lr"_a, "base_radius"_a)
        .def_rw("width", &fe::SOMTrainingConfig::width)
        .def_rw("height", &fe::SOMTrainingConfig::height)
        .def_rw("epochs", &fe::SOMTrainingConfig::epochs)
        .def_rw("base_lr", &fe::SOMTrainingConfig::baseLR)
        .def_rw("base_radius", &fe::SOMTrainingConfig::baseRadius)
        .def("validate", &fe::SOMTrainingConfig::validate, "Raises ValueError for a zero size/epochs or a non-finite or non-positive base_lr/base_radius. Called again wherever a config is used.")
        .def("to_json", [](const fe::SOMTrainingConfig& c){ return jsonToString(c.toJson()); },
             "Compact JSON: {\"width\":..., \"height\":..., \"epochs\":..., \"baseLR\":..., \"baseRadius\":...}")
        .def_static("from_json", [](const std::string& json){
                return fe::SOMTrainingConfig(parseJsonObject(json, "SOMTrainingConfig"));
            }, "json"_a)
        .def("__repr__", [](const fe::SOMTrainingConfig& c){
            return "SOMTrainingConfig(width=" + std::to_string(c.width) +
                ", height=" + std::to_string(c.height) +
                ", epochs=" + std::to_string(c.epochs) +
                ", base_lr=" + std::to_string(c.baseLR) +
                ", base_radius=" + std::to_string(c.baseRadius) + ")";
        });

    nb::class_<fe::SpeciesMatch>(m, "SpeciesMatch",
        "Where a flower lands on a classifier's grid: the (x, y) node of its species and "
        "`distance`, how far the flower's feature vector is from that node's prototype "
        "(euclidean, 0 for a flower identical to the prototype). There is no absolute scale: "
        "compare the distances of several flowers to find the atypical ones.")
        .def(nb::init<>())
        .def(nb::init<std::size_t, std::size_t, double>(), "x"_a, "y"_a, "distance"_a)
        .def_rw("x", &fe::SpeciesMatch::x)
        .def_rw("y", &fe::SpeciesMatch::y)
        .def_rw("distance", &fe::SpeciesMatch::distance)
        .def("__repr__", [](const fe::SpeciesMatch& s){
                return "SpeciesMatch(x=" + std::to_string(s.x) + ", y=" + std::to_string(s.y)
                    + ", distance=" + std::to_string(s.distance) + ")";
            });

    nb::class_<fe::SOMEpoch>(m, "SOMEpoch",
        "What one epoch of FlowerClassifier.continue_training() did: `epoch` (its index in the whole decay), "
        "`learning_rate`, `radius` (grid cells) and `mean_distance`, the mean distance from each sample to its "
        "species' prototype while the epoch ran. The prototypes move during the epoch, so it differs a little from "
        "FlowerClassifier.evaluate().mean_quantization_error afterwards.")
        .def_ro("epoch", &fe::SOMEpoch::epoch)
        .def_ro("learning_rate", &fe::SOMEpoch::learningRate)
        .def_ro("radius", &fe::SOMEpoch::radius)
        .def_ro("mean_distance", &fe::SOMEpoch::meanDistance)
        .def("__repr__", [](const fe::SOMEpoch& e){
                return "SOMEpoch(epoch=" + std::to_string(e.epoch) + ", learning_rate=" + std::to_string(e.learningRate)
                    + ", radius=" + std::to_string(e.radius) + ", mean_distance=" + std::to_string(e.meanDistance) + ")";
            });

    nb::class_<fe::SOMTrainingReport>(m, "SOMTrainingReport",
        "What FlowerClassifier.continue_training() did: `epochs`, one SOMEpoch per epoch that ran; `skipped`, the "
        "indices of the samples that were skipped (wrong size / non-finite values); and `node_counts`, the samples "
        "won by each grid node in the last epoch (flat index y * width + x).")
        .def_ro("epochs", &fe::SOMTrainingReport::epochs)
        .def_ro("skipped", &fe::SOMTrainingReport::skipped)
        .def_ro("node_counts", &fe::SOMTrainingReport::nodeCounts)
        .def("__repr__", [](const fe::SOMTrainingReport& r){
                return "SOMTrainingReport(epochs=" + std::to_string(r.epochs.size()) + ", skipped="
                    + std::to_string(r.skipped.size()) + ")";
            });

    nb::class_<fe::FlowerClassifier>(m, "FlowerClassifier",
        "A trained SOM species classifier: the underlying SOM plus the "
        "Environment it was trained under (documentation only, see "
        "Environment). Build one with train_flower_classifier() rather than "
        "this constructor directly unless you specifically want an "
        "untrained classifier to call .som-style training on by hand -- "
        "this constructor gives you the shape only, not a trained SOM.")
        .def(nb::init<std::size_t, std::size_t, fe::Environment>(),
             "width"_a, "height"_a, "environment"_a)
        .def_prop_ro("width", [](const fe::FlowerClassifier& c){ return c.som.getWidth(); },
             "Grid width, in nodes.")
        .def_prop_ro("height", [](const fe::FlowerClassifier& c){ return c.som.getHeight(); },
             "Grid height, in nodes.")
        .def_ro_static("silhouette_size", &fe::FlowerClassifier::silhouetteSize,
             "The N in the NxN silhouette that makes up the first input_dim-11 "
             "feature dimensions (16).")
        .def_ro_static("input_dim", &fe::FlowerClassifier::inputDim,
             "Feature vector length every sample must have (267): "
             "silhouette_size**2 silhouette values + 11 normalized Stats fields.")
        .def_static("stat_names", &fe::FlowerClassifier::statNames,
             "The 11 Stats field names making up the last 11 feature "
             "dimensions, in that fixed order.")
        .def_rw("environment", &fe::FlowerClassifier::environment)
        .def("get_prototype", [](const fe::FlowerClassifier& c, std::size_t x, std::size_t y){
                return c.som.getPrototype(x, y);
            }, "x"_a, "y"_a,
            "The learned feature vector at grid node (x, y) -- same shape as "
            "extract_flower_features()'s output (first silhouette_size**2 "
            "values reshape to a silhouette_size x silhouette_size image, "
            "last 11 are the normalized Stats fields, see stat_names()). "
            "Raises IndexError if x/y are out of range.")
        .def("classify",
            static_cast<std::optional<fe::SpeciesMatch>(fe::FlowerClassifier::*)(const std::string&, const fe::Environment&) const>(&fe::FlowerClassifier::classify),
            "genome"_a, "environment"_a,
            "Finds the grid node whose learned prototype is closest to this "
            "flower's genome: a SpeciesMatch with its x, y and the distance. "
            "Returns None (not an error) if genome is malformed or its DNA is "
            "unusable.")
        .def("classify",
            static_cast<std::optional<fe::SpeciesMatch>(fe::FlowerClassifier::*)(const fe::Flower&, const fe::Environment&) const>(&fe::FlowerClassifier::classify),
            "flower"_a, "environment"_a,
            "Same as the genome-string overload, for when you already have "
            "a Flower object instead of its genome JSON.")
        .def("classify_features", &fe::FlowerClassifier::classifyFeatures, "features"_a,
            "Classifies an already-extracted feature vector (see "
            "extract_flower_features()), skipping genome parsing and "
            "rendering. Returns None if features doesn't have exactly "
            "input_dim values or contains a non-finite one.")
        .def("classify_flowers", [](const fe::FlowerClassifier& c, const std::string& generations_json, const fe::Environment& environment){
                return classifyFlowers(c, generations_json, environment);
            }, "generations_json"_a, "environment"_a,
            "Classifies every flower of a {\"Generation\": [...]} JSON batch, "
            "one entry per flower in input order: a SpeciesMatch, or None for a flower "
            "that can't be classified (same meaning as classify()). Raises "
            "RuntimeError if generations_json isn't parsable JSON.")
        .def("evaluate", [](const fe::FlowerClassifier& c, const std::vector<std::vector<double>>& samples){
                auto e = c.evaluate(samples);
                nb::dict d;
                d["mean_quantization_error"] = e.meanQuantizationError;
                d["samples_used"] = e.samplesUsed;
                d["samples_skipped"] = e.samplesSkipped;
                d["total_nodes"] = e.totalNodes;
                d["used_nodes"] = e.usedNodes;
                d["fraction_used"] = e.fractionUsed;
                d["busiest_node_count"] = e.busiestNodeCount;
                d["samples_per_node_mean"] = e.samplesPerNodeMean;
                d["node_counts"] = e.nodeCounts;
                return d;
            }, "samples"_a,
            "Measures how well this classifier fits already-extracted feature "
            "vectors: mean quantization error plus how they spread over the "
            "nodes (node_counts is row-major, index = y * width + x). "
            "Wrong-size or non-finite samples are counted in samples_skipped. "
            "Raises ValueError if no sample is usable.")
        .def("continue_training", [](fe::FlowerClassifier& c, const std::vector<std::vector<double>>& samples,
                std::size_t epochs, double base_lr, double base_radius, std::size_t epoch_offset, std::size_t total_epochs,
                nb::object on_epoch){
                    fe::SOMEpochCallback callback;
                    if(!on_epoch.is_none()){
                        if(!PyCallable_Check(on_epoch.ptr())){
                            throw nb::type_error("on_epoch must be callable or None");
                        }
                        callback = [&on_epoch](const fe::SOMEpoch& epoch){
                            nb::object result = on_epoch(epoch);
                            return !result.is(nb::bool_(false)); // only an explicit False stops
                        };
                    }
                    return c.train(samples, epochs, base_lr, base_radius, epoch_offset, total_epochs, callback);
            }, "samples"_a, "epochs"_a, "base_lr"_a, "base_radius"_a, "epoch_offset"_a = 0, "total_epochs"_a = 0,
            "on_epoch"_a = nb::none(),
            "Trains this *already-existing* classifier further, in place, "
            "instead of building a fresh one -- for fine-tuning an "
            "embedded/loaded classifier on a smaller, more specific "
            "population, or for sampling a quantization-error-vs-epochs "
            "curve by calling this repeatedly with the same samples (see "
            "tools/). Uses the same fixed scheduler pair as "
            "train_flower_classifier(); by default each call "
            "is a new decay from its own epoch 0, so two calls with epochs=10 "
            "are not one call with epochs=20. To split ONE decay over several "
            "calls pass the same base_lr/base_radius and total_epochs every "
            "time and advance epoch_offset by the epochs already done; the "
            "decay is the same as in a single call (the sample order is "
            "reshuffled per call, so the weights are not bit-identical). on_epoch, if given, is "
            "called with a SOMEpoch after every epoch, to show progress or to stop: returning "
            "False stops after that epoch, anything else (None included) goes on; an exception "
            "it raises stops the training and is raised from here, the epochs already done stay. "
            "Returns a SOMTrainingReport: one SOMEpoch per epoch that ran, the indices of the "
            "samples that were skipped (wrong size / non-finite values; always fewer than "
            "len(samples)) and the wins per node of the last epoch. Raises "
            "ValueError if base_lr or base_radius is not finite and > 0, or if no sample is usable.")
        .def("to_json", [](const fe::FlowerClassifier& c){ return jsonToString(c.toJson()); },
             "The full classifier bundle -- version, input shape, stat "
             "names, environment, and the trained SOM's own weights -- as "
             "compact JSON. This is what gets embedded as "
             "fe::resources::trainedSOM once you're happy with a trained "
             "classifier (see tools/).")
        .def_prop_rw("weight_decimals",
            [](const fe::FlowerClassifier& c){ return c.som.getWeightDecimals(); },
            [](fe::FlowerClassifier& c, std::optional<int> decimals){ c.som.setWeightDecimals(decimals); },
            "Rounds the weights to_json() writes to this many decimals (0 to 17) to make the JSON "
            "much shorter, None (the default) writes them exactly. A weight within half a unit of the "
            "last decimal of 0 or 1 is written as exactly 0 or 1. The rounding is absolute and only "
            "changes the JSON, the classifier keeps its exact weights, and it is not saved in the JSON: a "
            "classifier loaded with from_json() writes exact weights until it is set again. 4 decimals "
            "write about a quarter of the text for the embedded 12x8 classifier. Raises ValueError "
            "outside 0 to 17.")
        .def_static("from_json", [](const std::string& json){
                return fe::FlowerClassifier(parseJsonObject(json, "FlowerClassifier"));
            }, "json"_a);

    m.def("extract_flower_features", &fe::extractFlowerFeatures, "genome"_a, "environment"_a,
        "Builds a flower's FlowerClassifier.input_dim-dimensional SOM "
        "feature vector from its genome JSON: a silhouette_size x "
        "silhouette_size alpha-only silhouette (max-pool downsampled, "
        "rendered with the genome's own petals config -- not a separately-"
        "supplied one) followed by 11 fixed-bounds-normalized Stats fields "
        "(see FlowerClassifier.stat_names()). Every value is in [0, 1]. "
        "Raises ValueError if genome's DNA doesn't have enough usable "
        "outputs, RuntimeError if genome isn't valid JSON at all. A genome "
        "whose network overflows (for example a weight of 1e999 in the "
        "JSON) gives NaN values instead of an error: filter with "
        "math.isfinite before using the vector yourself; training and "
        "classification skip such a vector. Extraction is not bit-for-bit "
        "reproducible between calls (the raw render varies for some "
        "genomes); on 300 random flowers it never changed the species "
        "cell. When you classify, evaluate or fine-tune the same flowers "
        "more than once, extract once and reuse the vectors.");

    m.def("train_flower_classifier",
        static_cast<fe::FlowerClassifier(*)(const std::vector<std::vector<double>>&, const fe::Environment&, const fe::SOMTrainingConfig&)>(&trainFlowerClassifier),
        "samples"_a, "environment"_a, "training"_a,
        "Trains a fresh classifier from already-extracted feature vectors "
        "(see extract_flower_features()). Use this overload when you want "
        "control over what happens to a sample before training it lands in "
        "here -- filtering, caching, inspecting, deduplicating -- otherwise "
        "the other overload (a list of genome JSON strings) does the "
        "extraction for you.");
    m.def("train_flower_classifier",
        static_cast<fe::FlowerClassifier(*)(const std::string&, const fe::Environment&, const fe::SOMTrainingConfig&)>(&trainFlowerClassifier),
        "generations_json"_a, "environment"_a, "training"_a,
        "Trains a fresh classifier from a whole batch given as one "
        "{\"Generation\": [...]} JSON string -- the format this project "
        "already uses to move a flower collection between apps (see "
        "tools/dataset.py for building one from a directory of saved "
        "genomes). Entries that fail feature extraction are skipped and "
        "logged to stderr, not treated as an error.");

    m.def("get_trained_som", [](){ return fe::resources::trainedSOM; },
        "The classifier embedded in this build, as-is (fe::resources::trainedSOM).");
}

} // namespace fe_py
