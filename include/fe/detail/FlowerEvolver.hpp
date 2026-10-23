#ifndef FLOWER_EVOLVER_DETAIL_HPP
#define FLOWER_EVOLVER_DETAIL_HPP

#include <string>
#include <utility>
#include <vector>

#include <EvoAI.hpp>

#include <fe/config.hpp>
#include <fe/FlowerClassifier.hpp>
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
		 * this file, so nothing here needs to cross a DLL boundary.
		 * Tests link the shared library too, so they go through the public API
		 * (a test calling these fails to link on Windows/MinGW shared builds)
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
        /**
         * @brief feature vectors of the usable flowers of a {"Generation": [...]} batch;
         * unusable ones are dropped (and counted on stderr), so indexes do not line up
         * with the input, unlike impl_extractFlowerFeaturesBatch().
         * @throw std::runtime_error if generationsJson is not parsable JSON.
         * @throw std::invalid_argument if environment is invalid.
         */
        std::vector<std::vector<double>> impl_extractFeaturesFromGenerationsJson(const std::string& generationsJson, const fe::Environment& environment);
		/**
		 * @brief trains a SOM species classifier over a batch of already-
		 * extracted feature vectors (see fe::extractFlowerFeatures()).
		 * Deliberately doesn't take genomes itself and doesn't do its own
		 * skip-on-bad-DNA handling: extraction is decoupled from training
		 * so a caller can stream it (extract one flower's features,
		 * discard the flower, accumulate the small feature vector, repeat)
		 * rather than needing every flower resident in memory at once just
		 * to call this once -- the expensive thing to hold at scale is
		 * flowers/rendered images, not their feature vectors (267 doubles
		 * each, ~2MB for 1000 of them).
		 * @param samples const std::vector<std::vector<double>>& each of
		 * size fe::FlowerClassifier::inputDim. Samples of the wrong size or
		 * containing non-finite values are skipped and logged to cerr by
		 * EvoAI::SelfOrganizingMap::train() itself, not treated as an error.
		 * @param environment const fe::Environment& recorded on the
		 * returned classifier as documentation only -- not enforced at
		 * classify time (see fe::Environment).
		 * @param training const fe::SOMTrainingConfig& grid size + hyperparameters.
		 * @return fe::FlowerClassifier
		 */
		fe::FlowerClassifier impl_trainFlowerClassifier(const std::vector<std::vector<double>>& samples,
			const fe::Environment& environment, const fe::SOMTrainingConfig& training);
		/**
		 * @brief trains a SOM species classifier over a whole batch given
		 * as one JSON string, {"Generation": [{"dna": ..., "petals": ...}, ...]}
		 * (the wire format this project already uses to move a flower
		 * collection between apps). Meant primarily for the wasm binding:
		 * one call across the JS/wasm boundary rather than one per flower,
		 * with the same low peak-memory profile as the samples overload
		 * above -- each entry is parsed, turned into a feature vector, and
		 * dropped before the next one starts (ordinary C++ scoping, no
		 * special machinery), so only one flower's DNA/rendered silhouette
		 * is ever alive at a time, alongside the small (267 doubles each)
		 * accumulated feature vectors. Equally usable natively/from python
		 * (e.g. a training script working off a saved flower collection
		 * file); wasm just benefits from it more.
		 * @param generationsJson const std::string&
		 * @param environment const fe::Environment& used for every flower's
		 * fe::Stats, and recorded on the returned classifier (see the
		 * samples overload above).
		 * @param training const fe::SOMTrainingConfig& grid size + hyperparameters.
		 * @return fe::FlowerClassifier
		 * @throw std::runtime_error if generationsJson is malformed JSON entirely.
		 */
		fe::FlowerClassifier impl_trainFlowerClassifier(const std::string& generationsJson,
			const fe::Environment& environment, const fe::SOMTrainingConfig& training);
		/**
		 * @brief fine-tunes an existing classifier in place on a whole
		 * {"Generation": [...]} batch, see fe::FlowerClassifier::train().
		 * @param environment used for this batch's fe::Stats; deliberately not
		 * the classifier's recorded one, the batch may come from other conditions.
		 * @throw std::runtime_error if generationsJson is malformed JSON entirely.
		 * @throw std::invalid_argument if no flower in the batch is usable (the
		 * classifier is left unchanged), or baseLR/baseRadius are invalid.
		 */
		/**
		 * @brief the feature vector of every flower of a {"Generation": [...]} batch,
		 * see fe::extractFlowerFeatures(), one flower at a time.
		 * @param environment used for every flower's fe::Stats.
		 * @return one entry per flower, in input order; std::nullopt for a flower
		 * that can't be extracted, so the indexes always line up with the input
		 * (a caller that stores the vectors per flower needs that).
		 * @throw std::runtime_error if generationsJson is malformed JSON entirely.
		 * @throw std::invalid_argument if environment is invalid.
		 */
		std::vector<std::optional<std::vector<double>>> impl_extractFlowerFeaturesBatch(const std::string& generationsJson,
			const fe::Environment& environment);
		/**
		 * @brief fe::FlowerClassifier::evaluate() over a whole {"Generation": [...]}
		 * batch, features extracted one flower at a time like the train overloads.
		 * @param environment used for every flower's fe::Stats.
		 * @throw std::runtime_error if generationsJson is malformed JSON entirely.
		 * @throw std::invalid_argument if no flower is usable.
		 */
		fe::SOMEvaluation impl_evaluateFlowerClassifier(const fe::FlowerClassifier& classifier,
			const std::string& generationsJson, const fe::Environment& environment);
		/**
		 * @brief classifies every flower of a {"Generation": [...]} batch against
		 * an already-loaded classifier: one JSON parse for the whole batch instead
		 * of one per flower, and the classifier is loaded once by the caller.
		 * @param environment used for every flower's fe::Stats.
		 * @return one entry per flower, in input order; std::nullopt for a flower
		 * that can't be classified (same meaning as fe::FlowerClassifier::classify()),
		 * so indexes always line up with the input.
		 * @throw std::runtime_error if generationsJson is malformed JSON entirely.
		 */
		std::vector<std::optional<fe::SpeciesMatch>> impl_classifyFlowers(const fe::FlowerClassifier& classifier,
			const std::string& generationsJson, const fe::Environment& environment);
		void impl_fineTuneFlowerClassifier(fe::FlowerClassifier& classifier, const std::string& generationsJson,
			const fe::Environment& environment, std::size_t epochs, double baseLR, double baseRadius);
	}
}

#endif // FLOWER_EVOLVER_DETAIL_HPP
