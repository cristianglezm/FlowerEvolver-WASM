#ifndef FLOWER_EVOLVER_SOM_TRAINING_REPORT_HPP
#define FLOWER_EVOLVER_SOM_TRAINING_REPORT_HPP

#include <cstddef>
#include <functional>
#include <vector>

#include <fe/config.hpp>

namespace fe{
    /**
     *  @brief what one epoch of fe::FlowerClassifier::train() did.
     */
    struct FE_API SOMEpoch final{
        /// index of the epoch in the whole decay (the epochOffset of the call plus the epochs before it in the call).
        std::size_t epoch = 0;
        /// learning rate used in this epoch.
        double learningRate = 0.0;
        /// neighborhood radius used in this epoch, in grid cells.
        double radius = 0.0;
        /**
         *  mean distance from each sample to its species' prototype while the epoch ran. It is
         *  free, but the prototypes move during the epoch, so it differs a little from what
         *  fe::FlowerClassifier::evaluate() measures afterwards (fe::SOMEvaluation::meanQuantizationError).
         */
        double meanDistance = 0.0;
    };

    /**
     *  @brief what fe::FlowerClassifier::train() did.
     */
    struct FE_API SOMTrainingReport final{
        /// one entry per epoch that ran.
        std::vector<SOMEpoch> epochs;
        /// indices in the samples given of those that were skipped (wrong size / non-finite values).
        std::vector<std::size_t> skipped;
        /// samples won by each grid node in the last epoch, flat index y * width + x.
        std::vector<std::size_t> nodeCounts;
    };

    /// called after every epoch of fe::FlowerClassifier::train(); return false to stop after that epoch.
    using SOMEpochCallback = std::function<bool(const SOMEpoch&)>;
}

#endif // FLOWER_EVOLVER_SOM_TRAINING_REPORT_HPP
