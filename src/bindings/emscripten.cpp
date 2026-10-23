#include <emscripten/bind.h>

#include <fe/detail/FlowerEvolver.hpp>
#include <fe/FlowerClassifier.hpp>
#include <fe/Image.hpp>
#include <fe/Resources.hpp>
#include <JsonBox.h>
#include <fe/config.hpp>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <optional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace{
	/// Largest SOM the wasm build will train. The heap is a fixed 30 MB (-sTOTAL_MEMORY in
	/// CMakeLists.txt, no growth) and training peaks at about 40 KB per node, because toJson()
	/// of the weights exists several times at once as text (measured natively: 16x16 ~10 MB,
	/// 16x32 ~20 MB, 32x32 ~40 MB). 256 nodes leaves room for the 3D generation that shares the
	/// heap; raise it together with -sTOTAL_MEMORY.
	constexpr std::size_t maxGridNodes = 256;

	/**
	 * @brief encodes an fe::Image straight to a base64 PNG data URL, in memory.
	 */
	std::string toBase64Png(const fe::Image& image){
		return "data:image/png;base64," + fe::encodeToBase64(fe::encodeImageToPngInMemory(image));
	}

	/**
	 * @brief parses the {"Flower": {"dna": {"genomes": [...]}}} wire format
	 * the JS side sends back in (genome strings produced by genomeStringOf()
	 * below, or anything a consumer built to the same shape) and returns
	 * the DNA it contains.
	 * @param label used only for the error message, e.g. "flower", "flower1".
	 * @throw std::invalid_argument if flowerJson can't be parsed or has no
	 * "Flower"/"dna" key -- same failure mode the pre-refactor bindings had.
	 */
	fe::DNA parseDna(const std::string& flowerJson, const char* label){
		JsonBox::Value v;
		v.loadFromString(flowerJson);
		if(v["Flower"]["dna"].isNull()){
			throw std::invalid_argument(std::string("error, invalid ") + label + ", could not parse data.");
		}
		return fe::DNA(v["Flower"]["dna"].getObject());
	}

	std::string genomeStringOf(const fe::Flower& flower){
		JsonBox::Value v;
		v["Flower"] = flower.toJson();
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}

	/// bundles a genome string + rendered image into the
	/// {"genome": ..., "image": ...} JSON FEService expects.
	std::string toJsResult(const std::string& genome, const fe::Image& image){
		JsonBox::Value v;
		v["genome"] = genome;
		v["image"] = toBase64Png(image);
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}
	std::string toJsResult(const fe::Flower& flower){
		return toJsResult(genomeStringOf(flower), flower.petals.image);
	}

	std::string makeFlower(int radius, int numLayers, float P, float bias){
		return toJsResult(fe::detail::impl_makeFlower(radius, numLayers, P, bias));
	}
	std::string makePetals(int radius, int numLayers, float P, float bias){
		return toJsResult(fe::detail::impl_makePetals(radius, numLayers, P, bias));
	}
	std::string makePetalLayer(int radius, int numLayers, float P, float bias, int layer){
		return toJsResult(fe::detail::impl_makePetalLayer(radius, numLayers, P, bias, layer));
	}
	std::string makeStem(int radius, int numLayers, float P, float bias){
		return toJsResult(fe::detail::impl_makeStem(radius, numLayers, P, bias));
	}
	std::string drawFlower(const std::string& flower, int radius, int numLayers, float P, float bias){
		auto dna = parseDna(flower, "flower");
		return toBase64Png(fe::detail::impl_drawFlower(dna, radius, numLayers, P, bias));
	}
	std::string drawPetals(const std::string& flower, int radius, int numLayers, float P, float bias){
		auto dna = parseDna(flower, "flower");
		return toBase64Png(fe::detail::impl_drawPetals(dna, radius, numLayers, P, bias));
	}
	std::string drawPetalLayer(const std::string& flower, int radius, int numLayers, float P, float bias, int layer){
		auto dna = parseDna(flower, "flower");
		return toBase64Png(fe::detail::impl_drawPetalLayer(dna, radius, numLayers, P, bias, layer));
	}
	std::string reproduce(const std::string& flower1, const std::string& flower2, int radius, int numLayers, float P, float bias){
		auto dna1 = parseDna(flower1, "flower1");
		auto dna2 = parseDna(flower2, "flower2");
		return toJsResult(fe::detail::impl_reproduce(dna1, dna2, radius, numLayers, P, bias));
	}
	std::string mutate(const std::string& original, int radius, int numLayers, float P, float bias,
			float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate,
			float enableRate, float disableRate, float actTypeRate){
		auto dna = parseDna(original, "original");
		return toJsResult(fe::detail::impl_mutate(std::move(dna), radius, numLayers, P, bias,
			addNodeRate, addConnRate, removeConnRate, perturbWeightsRate,
			enableRate, disableRate, actTypeRate));
	}
	std::string make3DFlower(const std::string& genome, int radius, int numLayers, float P, float bias, const std::string& flowerId, const std::string& flowerParams){
		auto dna = parseDna(genome, "flower genome");
		return fe::detail::impl_make3DFlower(dna, radius, numLayers, P, bias, flowerId, flowerParams);
	}
	/// Returns a JSON string shaped `{"stats": FlowerStats}` - FEService
	/// unwraps `.stats` for you.
	std::string getFlowerStats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType){
		auto stats = fe::detail::impl_getFlowerStats(genome, humidity, temperature, altitude, terrainType);
		JsonBox::Value v;
		v["stats"] = stats.toJson();
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}
	std::string getVersion(){
		return fe::version();
	}
	std::string getCommitHash(){
		return fe::commitHash();
	}
	std::string getVersionString(){
		return fe::versionString();
	}
	std::string getExceptionMessage(int exceptionPtr){
		return std::string(reinterpret_cast<std::exception*>(exceptionPtr)->what());
	}

	/**
	 * @brief parses a JSON-serialized fe::Environment -- the same shape
	 * fe::Environment::toJson() produces: {"humidity":...,
	 * "temperature":...,"altitude":...,"terrainType":...}. Fields are
	 * defaulted, not required (see fe::Environment(JsonBox::Object)), so
	 * this only throws if environmentJson isn't valid JSON at all.
	 */
	fe::Environment parseEnvironment(const std::string& environmentJson){
		JsonBox::Value v;
		v.loadFromString(environmentJson);
		return fe::Environment(v.getObject());
	}

	/**
	 * @brief loads a fe::FlowerClassifier from its JSON, or from the
	 * embedded fe::resources::trainedSOM if classifierJson is empty -- same
	 * "" for defaults convention make3DFlower's flowerParams already uses.
	 * @throw whatever fe::FlowerClassifier(JsonBox::Object)/JsonBox itself
	 * throws on malformed input -- including if classifierJson is "" and
	 * fe::resources::trainedSOM hasn't been populated with a real trained
	 * classifier yet (see tools/README.md).
	 */
	fe::FlowerClassifier loadClassifierOrDefault(const std::string& classifierJson){
		JsonBox::Value v;
		v.loadFromString(classifierJson.empty() ? fe::resources::trainedSOM : classifierJson);
		return fe::FlowerClassifier(v.getObject());
	}

	/// `{"x": ..., "y": ..., "distance": ...}` for a species match. A distance that overflowed to infinity
	/// (a classifier with absurd weights) is written as the largest finite double: infinity is not JSON.
	JsonBox::Value speciesMatchToJson(const fe::SpeciesMatch& match){
		JsonBox::Object o;
		o["x"] = JsonBox::Value(static_cast<int>(match.x));
		o["y"] = JsonBox::Value(static_cast<int>(match.y));
		o["distance"] = JsonBox::Value(std::isfinite(match.distance) ? match.distance : std::numeric_limits<double>::max());
		return JsonBox::Value(o);
	}

	/// Returns `{"match": {x, y, distance}}`, or `{"match": null}` for a genome that cannot be classified
	/// (not an error; a bad classifierJson/environmentJson still throws).
	/// classifierJson may be "" -- see loadClassifierOrDefault().
	std::string classifyFlower(const std::string& classifierJson, const std::string& genome, const std::string& environmentJson){
		auto classifier = loadClassifierOrDefault(classifierJson);
		auto environment = parseEnvironment(environmentJson);
		auto match = classifier.classify(genome, environment);
		JsonBox::Value v;
		if(match.has_value()){
			v["match"] = speciesMatchToJson(*match);
		}else{
			v["match"] = JsonBox::Value(); // null
		}
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}

	/// Returns the fe::SOMEvaluation of the classifier over the batch as JSON,
	/// see fe::FlowerClassifier::evaluate(). classifierJson may be "" -- see loadClassifierOrDefault().
	std::string evaluateClassifier(const std::string& classifierJson, const std::string& generationsJson, const std::string& environmentJson){
		auto classifier = loadClassifierOrDefault(classifierJson);
		auto environment = parseEnvironment(environmentJson);
		auto evaluation = fe::detail::impl_evaluateFlowerClassifier(classifier, generationsJson, environment);
		std::stringstream ss;
		evaluation.toJson().writeToStream(ss, false, true);
		return ss.str();
	}

	/// Returns `{"matches": [{x, y, distance} | null, ...]}`, one entry per flower of
	/// the {"Generation": [...]} batch, in order; null has the same meaning as
	/// in classifyFlower(). classifierJson may be "" -- see loadClassifierOrDefault().
	std::string classifyFlowers(const std::string& classifierJson, const std::string& generationsJson, const std::string& environmentJson){
		auto classifier = loadClassifierOrDefault(classifierJson);
		auto environment = parseEnvironment(environmentJson);
		auto matches = fe::detail::impl_classifyFlowers(classifier, generationsJson, environment);
		JsonBox::Array out;
		out.reserve(matches.size());
		for(auto& match : matches){
			out.emplace_back(match.has_value() ? speciesMatchToJson(*match) : JsonBox::Value()); // null
		}
		JsonBox::Value v;
		v["matches"] = JsonBox::Value(out);
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}

	/// Feature vectors as JSON: an array whose entries are an array of numbers, or null.
	std::vector<std::optional<std::vector<double>>> parseFeatures(const std::string& featuresJson){
		JsonBox::Value v;
		v.loadFromString(featuresJson);
		std::vector<std::optional<std::vector<double>>> out;
		for(auto& entry : v.getArray()){
			if(entry.isArray()){
				std::vector<double> vec;
				vec.reserve(entry.getArray().size());
				for(auto& n : entry.getArray()){
					vec.push_back(n.tryGetDouble(std::numeric_limits<double>::quiet_NaN()));
				}
				out.emplace_back(std::move(vec));
			}else{
				out.emplace_back(std::nullopt);
			}
		}
		return out;
	}

	/// The usable vectors only, for training and evaluating (nulls are dropped).
	std::vector<std::vector<double>> usableFeatures(const std::string& featuresJson){
		std::vector<std::vector<double>> out;
		for(auto& f : parseFeatures(featuresJson)){
			if(f){
				out.emplace_back(std::move(*f));
			}
		}
		return out;
	}

	/// Extracts every flower's feature vector once, so that classifying, evaluating and
	/// fine-tuning can then run on the vectors without rendering the flowers again.
	/// Returns `{"features": [[...] | null, ...]}`, one entry per flower in order; null for
	/// a flower that can't be extracted or whose vector has a NaN/Inf (a network that overflowed),
	/// which JsonBox would write as `nan` and JSON.parse() would reject for the whole batch.
	std::string extractFeatures(const std::string& generationsJson, const std::string& environmentJson){
		auto environment = parseEnvironment(environmentJson);
		auto batch = fe::detail::impl_extractFlowerFeaturesBatch(generationsJson, environment);
		JsonBox::Array out;
		out.reserve(batch.size());
		for(auto& features : batch){
			if(features && std::all_of(features->begin(), features->end(), [](double d){ return std::isfinite(d); })){
				JsonBox::Array vec;
				vec.reserve(features->size());
				for(double d : *features){
					vec.emplace_back(JsonBox::Value(d));
				}
				out.emplace_back(JsonBox::Value(vec));
			}else{
				out.emplace_back(JsonBox::Value());
			}
		}
		JsonBox::Value v;
		v["features"] = JsonBox::Value(out);
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}

	/// `{"matches": [{x, y, distance} | null, ...]}`, one entry per input entry; null for a null or
	/// unusable vector. classifierJson may be "" -- see loadClassifierOrDefault().
	std::string classifyFeatures(const std::string& classifierJson, const std::string& featuresJson){
		auto classifier = loadClassifierOrDefault(classifierJson);
		JsonBox::Array out;
		for(auto& features : parseFeatures(featuresJson)){
			std::optional<fe::SpeciesMatch> match;
			if(features){
				match = classifier.classifyFeatures(*features);
			}
			out.emplace_back(match ? speciesMatchToJson(*match) : JsonBox::Value());
		}
		JsonBox::Value v;
		v["matches"] = JsonBox::Value(out);
		std::stringstream ss;
		v.writeToStream(ss, false, true);
		return ss.str();
	}

	/// fe::SOMEvaluation as JSON over already-extracted vectors (nulls are dropped).
	std::string evaluateFeatures(const std::string& classifierJson, const std::string& featuresJson){
		auto classifier = loadClassifierOrDefault(classifierJson);
		std::stringstream ss;
		classifier.evaluate(usableFeatures(featuresJson)).toJson().writeToStream(ss, false, true);
		return ss.str();
	}

	/// fine-tunes on already-extracted vectors (nulls are dropped), see
	/// fe::FlowerClassifier::train() for epochOffset/totalEpochs: calling this once per
	/// epoch with a growing epochOffset runs one decay and lets the caller report progress.
	/// Returns the classifier's JSON like fineTune().
	std::string fineTuneFeatures(const std::string& classifierJson, const std::string& featuresJson,
			int epochs, double baseLR, double baseRadius, int epochOffset, int totalEpochs){
		auto classifier = loadClassifierOrDefault(classifierJson);
		// negatives from JS must not reach the size_t casts: they would wrap
		classifier.train(usableFeatures(featuresJson), epochs <= 0 ? 1u : static_cast<std::size_t>(epochs), baseLR, baseRadius,
			epochOffset <= 0 ? 0u : static_cast<std::size_t>(epochOffset), totalEpochs <= 0 ? 0u : static_cast<std::size_t>(totalEpochs));
		std::stringstream ss;
		JsonBox::Value(classifier.toJson()).writeToStream(ss, false, true);
		return ss.str();
	}

	/// Trains a fresh classifier from a whole {"Generation": [...]} batch. Returns the
	/// classifier's JSON directly, not wrapped in an envelope like classifyFlower()'s result.
	std::string trainFlowerClassifier(const std::string& generationsJson, const std::string& environmentJson, const std::string& trainingConfigJson){
		auto environment = parseEnvironment(environmentJson);
		JsonBox::Value trainingV;
		trainingV.loadFromString(trainingConfigJson);
		fe::SOMTrainingConfig training(trainingV.getObject());
		// wasm only: native and Python have no limit. Checked before anything is allocated,
		// width * height * inputDim doubles are allocated up front.
		if(training.width > maxGridNodes || training.height > maxGridNodes || training.width * training.height > maxGridNodes){
			throw std::invalid_argument("trainFlowerClassifier - the wasm build limits the grid to " + std::to_string(maxGridNodes) + " nodes");
		}
		auto classifier = fe::detail::impl_trainFlowerClassifier(generationsJson, environment, training);
		std::stringstream ss;
		JsonBox::Value(classifier.toJson()).writeToStream(ss, false, true);
		return ss.str();
	}

	/// Fine-tunes a classifier (classifierJson "" = the embedded one) on a new
	/// {"Generation": [...]} batch and returns its JSON.
	///
	/// Takes its own environmentJson: the batch's fe::Stats depend on it, and the classifier's
	/// recorded environment is only what it was trained under.
	std::string fineTune(const std::string& classifierJson, const std::string& generationsJson, const std::string& environmentJson,
			int epochs, double baseLR, double baseRadius){
		auto classifier = loadClassifierOrDefault(classifierJson);
		auto environment = parseEnvironment(environmentJson);
		// negative from JS must not reach the size_t cast: it would wrap to SIZE_MAX
		std::size_t safeEpochs = epochs <= 0 ? 1u : static_cast<std::size_t>(epochs);
		fe::detail::impl_fineTuneFlowerClassifier(classifier, generationsJson, environment, safeEpochs, baseLR, baseRadius);

		std::stringstream ss;
		JsonBox::Value(classifier.toJson()).writeToStream(ss, false, true);
		return ss.str();
	}

	/// The classifier embedded in this wasm binary, as-is.
	std::string getTrainedSOM(){
		return fe::resources::trainedSOM;
	}
}

EMSCRIPTEN_BINDINGS(makeFlower){
    emscripten::function("makeFlower", &makeFlower);
}
EMSCRIPTEN_BINDINGS(make3DFlower){
    emscripten::function("make3DFlower", &make3DFlower);
}
EMSCRIPTEN_BINDINGS(makePetals){
    emscripten::function("makePetals", &makePetals);
}
EMSCRIPTEN_BINDINGS(makePetalLayer){
    emscripten::function("makePetalLayer", &makePetalLayer);
}
EMSCRIPTEN_BINDINGS(makeStem){
    emscripten::function("makeStem", &makeStem);
}
EMSCRIPTEN_BINDINGS(drawFlower){
    emscripten::function("drawFlower", &drawFlower);
}
EMSCRIPTEN_BINDINGS(drawPetals){
    emscripten::function("drawPetals", &drawPetals);
}
EMSCRIPTEN_BINDINGS(drawPetalLayer){
    emscripten::function("drawPetalLayer", &drawPetalLayer);
}
EMSCRIPTEN_BINDINGS(reproduce){
    emscripten::function("reproduce", &reproduce);
}
EMSCRIPTEN_BINDINGS(mutate){
    emscripten::function("mutate", &mutate);
}
EMSCRIPTEN_BINDINGS(getFlowerStats){
    emscripten::function("getFlowerStats", &getFlowerStats);
}
EMSCRIPTEN_BINDINGS(classifyFlower){
    emscripten::function("classifyFlower", &classifyFlower);
}
EMSCRIPTEN_BINDINGS(featureBasedClassifier){
    emscripten::function("extractFeatures", &extractFeatures);
    emscripten::function("classifyFeatures", &classifyFeatures);
    emscripten::function("evaluateFeatures", &evaluateFeatures);
    emscripten::function("fineTuneFeatures", &fineTuneFeatures);
}
EMSCRIPTEN_BINDINGS(evaluateClassifier){
    emscripten::function("evaluateClassifier", &evaluateClassifier);
}
EMSCRIPTEN_BINDINGS(classifyFlowers){
    emscripten::function("classifyFlowers", &classifyFlowers);
}
EMSCRIPTEN_BINDINGS(trainFlowerClassifier){
    emscripten::function("trainFlowerClassifier", &trainFlowerClassifier);
}
EMSCRIPTEN_BINDINGS(fineTune){
    emscripten::function("fineTune", &fineTune);
}
EMSCRIPTEN_BINDINGS(getTrainedSOM){
    emscripten::function("getTrainedSOM", &getTrainedSOM);
}
EMSCRIPTEN_BINDINGS(getVersion){
    emscripten::function("getVersion", &getVersion);
}
EMSCRIPTEN_BINDINGS(getCommitHash){
    emscripten::function("getCommitHash", &getCommitHash);
}
EMSCRIPTEN_BINDINGS(getVersionString){
    emscripten::function("getVersionString", &getVersionString);
}
EMSCRIPTEN_BINDINGS(getExceptionMessage) {
    emscripten::function("getExceptionMessage", &getExceptionMessage);
};
