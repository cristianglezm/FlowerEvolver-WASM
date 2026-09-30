#ifndef FLOWER_EVOLVER_FLOWER_CLASSIFIER_HPP
#define FLOWER_EVOLVER_FLOWER_CLASSIFIER_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <EvoAI.hpp>
#include <JsonBox.h>

#include <fe/config.hpp>
#include <fe/Environment.hpp>
#include <fe/Flower.hpp>
#include <fe/SOMEvaluation.hpp>
#include <fe/SOMTrainingConfig.hpp>
#include <fe/SOMTrainingReport.hpp>

namespace fe{
    /**
     *  @brief builds a flower's SOM feature vector: FlowerClassifier::inputDim
     *  values, a silhouetteSize x silhouetteSize silhouette (petals-only render,
     *  alpha channel, max-pool downsampled) followed by
     *  FlowerClassifier::statNames()'s fe::Stats fields normalized to fixed
     *  bounds, in that order.
     *
     *  Takes the genome as a string because fe::Stats is string-based: the
     *  string is passed to it unchanged and parsed once here for the render.
     *
     *  @param [in] genome      "{"Flower": {"dna": ..., "petals": ...}}"
     *  @param [in] environment used for this flower's fe::Stats.
     *  @return std::vector<double> size FlowerClassifier::inputDim.
     *  A missing or malformed "petals" section is not an error: its fields fall
     *  back to the defaults, and radius/numLayers are clamped like fe::Petals does.
     *  @throw std::runtime_error if genome is not parsable JSON.
     *  @throw std::invalid_argument if the DNA has fewer than 2 genomes or doesn't
     *  have enough usable outputs to render the silhouette and compute the stats.
     *  @note weights that overflow the network (e.g. 1e999 in the JSON) give NaN
     *  values in the vector instead of an error; train()/classify() skip such a
     *  vector, so filter with std::isfinite if the vector is used directly.
     */
    FE_API std::vector<double> extractFlowerFeatures(const std::string& genome, const fe::Environment& environment);

    /**
     *  @brief where a flower lands on a classifier's grid: its species and how typical it is of it.
     */
    struct FE_API SpeciesMatch final{
        /// column of the best matching unit on the SOM grid.
        std::size_t x = 0u;
        /// row of the best matching unit on the SOM grid.
        std::size_t y = 0u;
        /**
         *  distance from the flower's feature vector to the species' prototype
         *  (euclidean over the vector's 267 values): 0 for a flower identical to
         *  the prototype, the larger the more atypical of its species. There is
         *  no absolute scale, compare it with the distances of other flowers.
         */
        double distance = 0.0;
    };

    /**
     *  @brief a SOM species classifier: the trained
     *  EvoAI::SelfOrganizingMap<> plus the fe::Environment it was trained
     *  under (a record only, see fe::Environment).
     */
    struct FE_API FlowerClassifier final{
        /// side of the square silhouette in the feature vector.
        static constexpr std::size_t silhouetteSize = 16;
        /**
         *  @brief the length every feature vector passed to train()/classify()
         *  must have: the silhouette plus 11 normalized fe::Stats values.
         *  Fixed, since a classifier trained on one layout can't classify another.
         */
        static constexpr std::size_t inputDim = (silhouetteSize * silhouetteSize) + 11;
        /**
         *  @brief the fe::Stats fields making up the last 11 dimensions of
         *  the feature vector, in that fixed order. `sex` is excluded
         *  (categorical, not a morphology/stat scalar).
         *  @return const std::vector<std::string>&
         */
        static const std::vector<std::string>& statNames() noexcept;

        /**
         *  @brief fresh, untrained classifier of the given grid size, see train().
         *  @param [in] width       grid width, must be > 0
         *  @param [in] height      grid height, must be > 0
         *  @param [in] environment a record only, see fe::Environment.
         *  @throw std::invalid_argument if width or height is 0 (see EvoAI::SelfOrganizingMap).
         */
        FlowerClassifier(std::size_t width, std::size_t height, fe::Environment environment);
        /**
         *  @brief loads a classifier saved with toJson().
         *  @param [in] o JsonBox::Object
         *  @throw std::invalid_argument if malformed (see EvoAI::SelfOrganizingMap(JsonBox::Object)),
         *  or if its input dimension doesn't match FlowerClassifier::inputDim
         *  (trained with a different feature layout); it would otherwise load
         *  and then crash or match meaninglessly in classify().
         */
        explicit FlowerClassifier(JsonBox::Object o);
        /**
         *  @brief classifies a single flower from its genome string.
         *  Does extractFlowerFeatures() and the SOM lookup in one call.
         *  @param [in] genome      "{"Flower": {"dna": ..., "petals": ...}}"
         *  @param [in] environment used for this flower's fe::Stats. Not
         *  required or expected to match this.environment -- see fe::Environment.
         *  @return std::optional<SpeciesMatch> the best matching unit on the SOM
         *  grid and the distance to it, or empty if genome is malformed or its
         *  DNA is unusable.
         */
        std::optional<SpeciesMatch> classify(const std::string& genome, const fe::Environment& environment) const;
        /**
         *  @brief classifies an already-built flower, for a native caller that
         *  holds a fe::Flower instead of its genome string. Delegates to the
         *  string overload; flower.petals.image is not reused because it was
         *  drawn with the trunk for display, not the petals-only render the
         *  classifier was trained on.
         *  @param [in] flower      the flower to classify.
         *  @param [in] environment used for this flower's fe::Stats.
         *  @return see the string overload.
         */
        std::optional<SpeciesMatch> classify(const fe::Flower& flower, const fe::Environment& environment) const;
        /**
         *  @brief classifies an already-extracted feature vector, see extractFlowerFeatures().
         *  @param [in] features a vector of inputDim finite values.
         *  @return std::optional<SpeciesMatch> empty if the vector has the wrong size
         *  or a value that is not finite.
         */
        std::optional<SpeciesMatch> classifyFeatures(const std::vector<double>& features) const;
        /**
         *  @brief trains this classifier in place on already-extracted
         *  feature vectors (see extractFlowerFeatures()). Fine-tuning a
         *  trained classifier is the same call. Every binding delegates here,
         *  so this is the only place the decay schedule lives.
         *
         *  Learning rate and radius both decay exponentially from their base
         *  value to a fixed floor (0.01 and 0.5) over `totalEpochs`.
         *
         *  One decay can be split across several calls (progress reporting,
         *  a time budget): pass the same baseLR/baseRadius/totalEpochs each
         *  time and advance epochOffset by the epochs already done. The decay
         *  then matches a single call; the weights only match to rounding, and
         *  only when sample order does not matter (e.g. one sample), because
         *  each call reshuffles. A call without them starts a new decay at its
         *  own epoch 0, which is what fine-tuning on a new batch wants.
         *  @param [in] samples    each of size FlowerClassifier::inputDim.
         *  Unusable ones (wrong size, non-finite) are skipped; their indices are in
         *  the report.
         *  @param [in] epochs     epochs to run in this call; 0 trains nothing.
         *  @param [in] baseLR     learning rate at the start of the whole decay; finite and > 0.
         *  @param [in] baseRadius neighborhood radius at the start of the whole decay; finite and > 0.
         *  @param [in] epochOffset epochs of the decay already done by earlier calls.
         *  @param [in] totalEpochs epochs of the whole decay; 0 means epochOffset + epochs
         *  (this call is the last one, or the only one).
         *  @param [in] onEpoch    called after every epoch with what it did, to report progress
         *  or to stop: returning false stops after that epoch (the epoch is in the report).
         *  An exception it throws stops the training and reaches the caller, the classifier
         *  keeps the epochs already done.
         *  @return SOMTrainingReport one entry per epoch that ran, the indices of the skipped
         *  samples (always fewer than samples.size()) and the wins per node of the last epoch.
         *  @throw std::invalid_argument if baseLR or baseRadius is not finite and > 0,
         *  if epochOffset + epochs doesn't fit in totalEpochs, or if no sample
         *  is usable (samples empty or all skipped); the weights are left unchanged.
         */
        SOMTrainingReport train(const std::vector<std::vector<double>>& samples, std::size_t epochs, double baseLR, double baseRadius,
            std::size_t epochOffset = 0, std::size_t totalEpochs = 0, const SOMEpochCallback& onEpoch = {});
        /**
         *  @brief converts to JsonBox::Value.
         *  @return JsonBox::Value
         */
        /**
         *  @brief measures how well this classifier fits `samples`, see fe::evaluateSOM().
         *  @param [in] samples each of size FlowerClassifier::inputDim, see
         *  fe::extractFlowerFeatures(). Wrong-size or non-finite ones are
         *  counted in samplesSkipped, not an error.
         *  @return fe::SOMEvaluation
         *  @throw std::invalid_argument if no sample is usable (the mean
         *  would be undefined).
         */
        SOMEvaluation evaluate(const std::vector<std::vector<double>>& samples) const;
        JsonBox::Value toJson() const noexcept;
        // data
        EvoAI::SelfOrganizingMap<> som;
        fe::Environment environment;
    };

} // namespace fe


#endif // FLOWER_EVOLVER_FLOWER_CLASSIFIER_HPP
