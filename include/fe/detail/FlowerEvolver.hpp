#ifndef FLOWER_EVOLVER_DETAIL_HPP
#define FLOWER_EVOLVER_DETAIL_HPP

#include <string>

#include <EvoAI.hpp>

#include <fe/config.hpp>
#include <fe/Image.hpp>
#include <fe/Flower.hpp>
#include <fe/Stats.hpp>

namespace fe{
	namespace detail{
		/**
		 * @brief Shared generation logic.
		 * Not the public API, ``include/fe/FlowerEvolver.hpp`` is.
		 * Don't call these directly from consumer code. Deliberately NOT marked
		 * FE_API: every caller (src/bindings/native.cpp,
		 * src/bindings/emscripten.cpp) is compiled into the same target as
		 * this file, so nothing here needs to cross a DLL boundary
		 */

		/**
		 * @brief makes a flower.
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
		 */
		fe::Flower impl_makeFlower(int radius, int numLayers, float P, float bias) noexcept;
		/**
		 * @brief makes just the petals (no stem).
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
		 */
		fe::Flower impl_makePetals(int radius, int numLayers, float P, float bias) noexcept;
		/**
		 * @brief makes just one petal layer (no stem).
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @param layer int layer to paint
		 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
		 */
		fe::Flower impl_makePetalLayer(int radius, int numLayers, float P, float bias, int layer) noexcept;
		/**
		 * @brief makes just a stem (no petals).
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
		 */
		fe::Flower impl_makeStem(int radius, int numLayers, float P, float bias) noexcept;
		/**
		 * @brief renders the given genome.
		 * @param dna const fe::DNA& genome to render (needs at least 2 genomes)
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @return fe::Image the rendered image
		 * @throw std::invalid_argument if dna has fewer than 2 genomes.
		 */
		fe::Image impl_drawFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias);
		/**
		 * @brief renders the given genome's petals (no stem).
		 * @param dna const fe::DNA& genome to render (needs at least 2 genomes)
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @return fe::Image the rendered image
		 * @throw std::invalid_argument if dna has fewer than 2 genomes.
		 */
		fe::Image impl_drawPetals(const fe::DNA& dna, int radius, int numLayers, float P, float bias);
		/**
		 * @brief renders the given genome's petal layer (no stem).
		 * @param dna const fe::DNA& genome to render (needs at least 2 genomes)
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @param layer int layer to render
		 * @return fe::Image the rendered image
		 * @throw std::invalid_argument if dna has fewer than 2 genomes.
		 */
		fe::Image impl_drawPetalLayer(const fe::DNA& dna, int radius, int numLayers, float P, float bias, int layer);
		/**
		 * @brief makes a child from two genomes.
		 * @param dna1 const fe::DNA& father's genome
		 * @param dna2 const fe::DNA& mother's genome
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
		 * @throw std::runtime_error if dna1 and dna2 don't have the same number of genomes.
		 */
		fe::Flower impl_reproduce(const fe::DNA& dna1, const fe::DNA& dna2, int radius, int numLayers, float P, float bias);
		/**
		 * @brief mutates the given genome. `original` is taken by value and
		 * mutated in place internally -- the caller's own copy, if any, is
		 * left untouched.
		 * @param original fe::DNA genome to mutate (needs at least 2 genomes)
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @param addNodeRate float rate to add a new node
		 * @param addConnRate float rate to add a new connection
		 * @param removeConnRate float rate to remove an existing connection
		 * @param perturbWeightsRate float rate to change the weight of a connection
		 * @param enableRate float rate to enable a currently-disabled gene
		 * @param disableRate float rate to disable a currently-enabled gene
		 * @param actTypeRate float rate to change a neuron's activation function
		 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
		 * @throw std::invalid_argument if original has fewer than 2 genomes.
		 */
		fe::Flower impl_mutate(fe::DNA original, int radius, int numLayers, float P, float bias,
			float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate,
			float enableRate, float disableRate, float actTypeRate);
		/**
		 * @brief gets the flower stats from the DNA[0].
		 * @param genome const std::string& stringified flower.json
		 * @param humidity float 0.0 to 1.0
		 * @param temperature int temperature
		 * @param altitude int meters above sea
		 * @param terrainType int terrain type
		 * @return fe::Stats
		 * @throw std::invalid_argument if genome doesn't contain a DNA with at least 2 genomes.
		 * @throw std::runtime_error if genome is malformed JSON entirely.
		 */
		fe::Stats impl_getFlowerStats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType);
		/**
		 * @brief generates a 3D flower model in glTF format.
		 * @param dna const fe::DNA& genome to render (needs at least 2 genomes)
		 * @param radius int radius for the flower
		 * @param numLayers int how many layers it will have
		 * @param P float P parameter, controls roughly how many petals the flower can have.
		 * @param bias float bias
		 * @param flowerId A unique string identifier for this flower instance (used in group names).
		 * @param flowerParams std::string json fe::FlowerParameters for the 3d flower; "" for defaults.
		 * @return A std::string containing the 3D model in glTF format.
		 * @throw std::invalid_argument if dna has fewer than 2 genomes, or radius/numLayers/flowerId are invalid.
		 */
		std::string impl_make3DFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias, const std::string& flowerId, const std::string& flowerParams);
	}
}

#endif // FLOWER_EVOLVER_DETAIL_HPP
