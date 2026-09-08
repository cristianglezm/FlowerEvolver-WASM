/**
 * @brief Thin native (non-Emscripten) binding layer: every function here
 * is declared and fully documented in include/fe/FlowerEvolver.hpp
 */
#include <fe/FlowerEvolver.hpp>
#include <fe/detail/FlowerEvolver.hpp>

fe::Flower makeFlower(int radius, int numLayers, float P, float bias) noexcept{
	return fe::detail::impl_makeFlower(radius, numLayers, P, bias);
}

fe::Flower makePetals(int radius, int numLayers, float P, float bias) noexcept{
	return fe::detail::impl_makePetals(radius, numLayers, P, bias);
}

fe::Flower makePetalLayer(int radius, int numLayers, float P, float bias, int layer) noexcept{
	return fe::detail::impl_makePetalLayer(radius, numLayers, P, bias, layer);
}

fe::Flower makeStem(int radius, int numLayers, float P, float bias) noexcept{
	return fe::detail::impl_makeStem(radius, numLayers, P, bias);
}

fe::Image drawFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias){
	return fe::detail::impl_drawFlower(dna, radius, numLayers, P, bias);
}

fe::Image drawPetals(const fe::DNA& dna, int radius, int numLayers, float P, float bias){
	return fe::detail::impl_drawPetals(dna, radius, numLayers, P, bias);
}

fe::Image drawPetalLayer(const fe::DNA& dna, int radius, int numLayers, float P, float bias, int layer){
	return fe::detail::impl_drawPetalLayer(dna, radius, numLayers, P, bias, layer);
}

fe::Flower reproduce(const fe::DNA& dna1, const fe::DNA& dna2, int radius, int numLayers, float P, float bias){
	return fe::detail::impl_reproduce(dna1, dna2, radius, numLayers, P, bias);
}

fe::Flower mutate(fe::DNA original, int radius, int numLayers, float P, float bias,
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate,
					float enableRate, float disableRate, float actTypeRate){
	return fe::detail::impl_mutate(std::move(original), radius, numLayers, P, bias,
		addNodeRate, addConnRate, removeConnRate, perturbWeightsRate,
		enableRate, disableRate, actTypeRate);
}

fe::Stats getFlowerStats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType){
	return fe::detail::impl_getFlowerStats(genome, humidity, temperature, altitude, terrainType);
}

std::string make3DFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias, const std::string& flowerId, const std::string& flowerParams){
	return fe::detail::impl_make3DFlower(dna, radius, numLayers, P, bias, flowerId, flowerParams);
}
