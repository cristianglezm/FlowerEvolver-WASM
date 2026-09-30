#ifndef FLOWER_EVOLVER_SOM_TRAINING_CONFIG_HPP
#define FLOWER_EVOLVER_SOM_TRAINING_CONFIG_HPP

#include <cstddef>

#include <JsonBox.h>

#include <fe/config.hpp>

namespace fe{
    /**
     *  @brief SOM grid size and training hyperparameters.
     *
     *  JSON-serializable so a training pipeline can carry it across calls or
     *  messages. The defaults are placeholders, not tuned.
     */
    struct FE_API SOMTrainingConfig final{
        /**
         *  @brief default constructor, see the defaults below.
         */
        SOMTrainingConfig() noexcept = default;
        /**
         *  @brief constructor with all the parameters
         *  @throw std::invalid_argument if width/height/epochs == 0, or
         *  baseLR/baseRadius <= 0.
         */
        SOMTrainingConfig(std::size_t width, std::size_t height, std::size_t epochs, double baseLR, double baseRadius);
        /**
         *  @brief loads a JsonBox::Object. A missing field uses its default;
         *  a present but invalid one (<= 0 where > 0 is required) throws
         *  instead of falling back to the default.
         *  @param [in] o JsonBox::Object
         *  @throw std::invalid_argument per the above.
         */
        explicit SOMTrainingConfig(JsonBox::Object o);
        /**
         *  @brief the fields are public, so a value that was valid at
         *  construction can be changed afterwards; the code that consumes
         *  a config calls this again.
         *  @throw std::invalid_argument if width/height/epochs == 0, or
         *  baseLR/baseRadius is not finite and > 0.
         */
        void validate() const;
        /**
         *  @brief converts to JsonBox::Value
         *  @return JsonBox::Value
         */
        JsonBox::Value toJson() const noexcept;
        // data
        std::size_t width = 5;
        std::size_t height = 5;
        std::size_t epochs = 100;
        double baseLR = 0.5;
        double baseRadius = 2.5;
    };
} // namespace fe

#endif // FLOWER_EVOLVER_SOM_TRAINING_CONFIG_HPP
