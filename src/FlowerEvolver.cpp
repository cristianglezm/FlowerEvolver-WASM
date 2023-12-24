#include <FlowerEvolver.hpp>

void copyToCanvas(std::uint8_t* ptr, int w, int h){
	EM_ASM_({
		let data = Module.HEAPU8.slice($0, $0 + $1 * $2 * 4);
		let context = document.getElementById("canvas").getContext('2d');
		let imageData = context.getImageData(0, 0, $1, $2);
		imageData.data.set(data);
		context.putImageData(imageData, 0, 0);
    }, ptr, w, h);
}
std::string makeFlower(int radius, int numLayers, float P, float bias) noexcept{
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	fe::Flower flower({0.f, 0.f}, radius, numLayers, P, bias);
	auto size = flower.petals.image.getSize();
	copyToCanvas(flower.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = flower.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}


std::string reproduce(const std::string& flower1, const std::string& flower2, int radius, int numLayers, float P, float bias){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	JsonBox::Value v1;
	v1.loadFromString(flower1);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower1, could not parse data.");
	}
	JsonBox::Value v2;
	v2.loadFromString(flower2);
	if(v2["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid flower2, could not parse data.");
	}
	fe::DNA dna1(fe::DNA(v1["Flower"]["dna"].getObject()));
	fe::DNA dna2(fe::DNA(v2["Flower"]["dna"].getObject()));
	auto child = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, fe::DNA::reproduce(dna1, dna2));
	auto size = child.petals.image.getSize();
	copyToCanvas(child.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = child.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}

std::string mutate(const std::string& original, int radius, int numLayers, float P, float bias, 
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, 
					float enableRate, float disableRate, float actTypeRate){
	EvoAI::randomGen().setSeed(std::chrono::steady_clock::now().time_since_epoch().count());
	JsonBox::Value v1;
	v1.loadFromString(original);
	if(v1["Flower"]["dna"].isNull()){
		throw std::invalid_argument("error, invalid original, could not parse data.");
	}
	fe::DNA dna(fe::DNA(v1["Flower"]["dna"].getObject()));
	dna.mutate(fe::MutationRates(addNodeRate, addConnRate, removeConnRate, perturbWeightsRate, enableRate, disableRate, actTypeRate));
	auto mutated = fe::Flower({0.0, 0.0}, radius, numLayers, P, bias, std::move(dna));
	auto size = mutated.petals.image.getSize();
	copyToCanvas(mutated.petals.image.imageData.data(), size.x, size.y);
	std::stringstream ss;
	JsonBox::Value v;
	v["Flower"] = mutated.toJson();
	v.writeToStream(ss, false, true);
	return ss.str();
}
