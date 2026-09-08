#ifndef FLOWER_EVOLVER_HPP
#define FLOWER_EVOLVER_HPP

#include <string>

#include <fe/config.hpp>
#include <fe/Image.hpp>
#include <fe/Flower.hpp>
#include <fe/Stats.hpp>
#include <fe/detail/FlowerEvolver.hpp>

/**
 * @brief makes a flower.
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
 */
FE_API fe::Flower makeFlower(int radius, int numLayers, float P, float bias) noexcept;
/**
 * @brief Generates a 3D flower model as a glTF string.
 *
 * Orchestrates the process of generating a stem, processing 2D layer images
 * (finding contours, simplifying), generating 3D petal geometry for each layer,
 * and assembling the final glTF data.
 *
 * @param dna The genetic information used to draw the layers.
 * @param radius Initial radius parameter for fe::Petals constructor.
 * @param numLayers Number of layers parameter for fe::Petals constructor and loop control.
 * @param P P parameter for fe::Petals constructor.
 * @param bias Bias parameter for fe::Petals constructor.
 * @param flowerId A unique string identifier for this flower instance (used in group names).
 * @param flowerParams std::string json fe::FlowerParameters for the 3d flower.
 * @return A std::string containing the 3D model in GLTF format. Returns empty string on error.
 *         (GLTF is a text interchange format either way, so unlike the image
 *         functions below there's no more "native" form to return instead.)
 */
FE_API std::string make3DFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias, const std::string& flowerId, const std::string& flowerParams = "");
/**
 * @brief makes just the petals (no stem).
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
 */
FE_API fe::Flower makePetals(int radius, int numLayers, float P, float bias) noexcept;
/**
 * @brief makes just the petal layer (no stem).
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @param layer int layer layer to paint
 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
 */
FE_API fe::Flower makePetalLayer(int radius, int numLayers, float P, float bias, int layer) noexcept;
/**
 * @brief makes just the stem (no petals).
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
 */
FE_API fe::Flower makeStem(int radius, int numLayers, float P, float bias) noexcept;
/**
 * @brief renders the given genome.
 * 
 * @param dna const fe::DNA& genome to render
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return fe::Image the rendered image
 */
FE_API fe::Image drawFlower(const fe::DNA& dna, int radius, int numLayers, float P, float bias);
/**
 * @brief renders the given genome's petals (no stem).
 * 
 * @param dna const fe::DNA& genome to render
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return fe::Image the rendered image
 */
FE_API fe::Image drawPetals(const fe::DNA& dna, int radius, int numLayers, float P, float bias);
/**
 * @brief renders the given genome's petal layer (no stem).
 * 
 * @param dna const fe::DNA& genome to render (needs at least 2 genomes)
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @param layer int layer to render
 * @return fe::Image the rendered image
 */
FE_API fe::Image drawPetalLayer(const fe::DNA& dna, int radius, int numLayers, float P, float bias, int layer);
/**
 * @brief makes a child from two genomes.
 * 
 * @param dna1 const fe::DNA& father's genome
 * @param dna2 const fe::DNA& mother's genome
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
 */
FE_API fe::Flower reproduce(const fe::DNA& dna1, const fe::DNA& dna2, int radius, int numLayers, float P, float bias);
/**
 * @brief mutates the given genome. `original` is taken by value and mutated
 * in place internally - the caller's own copy, if any, is left untouched.
 * 
 * @param original fe::DNA genome to mutate
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @param addNodeRate float rate to determine what it mutates
 * @param addConnRate float rate to determine what it mutates
 * @param removeConnRate float rate to determine what it mutates
 * @param perturbWeightsRate float rate to determine what it mutates
 * @param enableRate float rate to determine what it mutates
 * @param disableRate float rate to determine what it mutates
 * @param actTypeRate float rate to determine what it mutates
 * @return fe::Flower -- .toJson() for the genome, .petals.image for the render
 */
FE_API fe::Flower mutate(fe::DNA original, int radius, int numLayers, float P, float bias, 
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, 
					float enableRate, float disableRate, float actTypeRate);
/**
 * @brief gets the flower stats from the DNA[0].
 * @param genome const std::string& stringified flower.json -- passed straight
 *        through to fe::Stats's own (pre-existing) string-based constructor,
 *        so there's no parse/round-trip to avoid here like there was in the
 *        functions above.
 * @param humidity float 0.0 to 1.0
 * @param temperature int temperature
 * @param altitude int meters above sea
 * @param terrainType int terrain type
 * @return fe::Stats
 */
FE_API fe::Stats getFlowerStats(const std::string& genome, float humidity, int temperature, int altitude, int terrainType);

#endif // FLOWER_EVOLVER_HPP
