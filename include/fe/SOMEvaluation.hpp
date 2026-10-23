#ifndef FLOWER_EVOLVER_SOM_EVALUATION_HPP
#define FLOWER_EVOLVER_SOM_EVALUATION_HPP

#include <cstddef>
#include <vector>

#include <EvoAI.hpp>
#include <JsonBox.h>

#include <fe/config.hpp>

namespace fe{
    /**
     *  @brief how well a trained SOM fits a set of samples, see fe::evaluateSOM()
     *  and fe::FlowerClassifier::evaluate().
     */
    struct FE_API SOMEvaluation final{
        /**
         *  @brief converts to JsonBox::Value, nodeCounts as a flat array.
         *  @return JsonBox::Value
         */
        JsonBox::Value toJson() const noexcept;
        /// mean Euclidean distance from each usable sample to its BMU's prototype.
        double meanQuantizationError = 0.0;
        /// usable samples, i.e. those the classifier could classify.
        std::size_t samplesUsed = 0;
        /// samples that were skipped (wrong size / non-finite values).
        std::size_t samplesSkipped = 0;
        std::size_t totalNodes = 0;
        /// nodes that are the BMU of at least one sample.
        std::size_t usedNodes = 0;
        double fractionUsed = 0.0;
        /// sample count of the most crowded node.
        std::size_t busiestNodeCount = 0;
        double samplesPerNodeMean = 0.0;
        /// how many samples landed on each node, row-major: index = y * width + x.
        std::vector<std::size_t> nodeCounts;
    };
    /**
     *  @brief measures how well a trained SOM fits `samples`: mean
     *  quantization error and how the samples spread over the nodes.
     *  Mirrors tools/som_trainer/evaluation.py. Read both when judging
     *  convergence: the error can look flat while the load is still being
     *  redistributed over the nodes.
     *
     *  Only depends on EvoAI::SelfOrganizingMap, not on flowers.
     *  @param [in] som the trained map.
     *  @param [in] samples each of size som's input dimension. Wrong-size or
     *  non-finite ones are counted in samplesSkipped, not an error.
     *  @return fe::SOMEvaluation
     *  @throw std::invalid_argument if no sample is usable (the mean would be undefined).
     */
    FE_API SOMEvaluation evaluateSOM(const EvoAI::SelfOrganizingMap<>& som, const std::vector<std::vector<double>>& samples);
}

#endif // FLOWER_EVOLVER_SOM_EVALUATION_HPP
