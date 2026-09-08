#include <emscripten/bind.h>

#include <fe/detail/FlowerEvolver.hpp>
#include <fe/Image.hpp>
#include <JsonBox.h>
#include <fe/config.hpp>

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace{
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
