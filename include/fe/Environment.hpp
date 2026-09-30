#ifndef FLOWER_EVOLVER_ENVIRONMENT_HPP
#define FLOWER_EVOLVER_ENVIRONMENT_HPP

#include <JsonBox.h>

#include <fe/config.hpp>

namespace fe{
    /**
     *  @brief the environment (humidity/temperature/altitude/terrainType)
     *  that fe::Stats is computed against.
     *
     *  A classifier's JSON stores it only as a record of what it was trained
     *  under; it is not checked at classify time because the game varies the
     *  environment per flower on purpose. `humidity` is only required to be
     *  finite: NaN would flow through fe::Stats into every feature that
     *  depends on it and poison a whole SOM from one sample.
     */
    struct FE_API Environment final{
        /**
         *  @brief default constructor
         */
        Environment() noexcept;
        /**
         *  @brief constructor with all the parameters
         *
         *  @param [in] humidity    float 0.0 to 1.0 by convention, not enforced; must be finite
         *  @param [in] temperature int temperature
         *  @param [in] altitude    int meters above sea
         *  @param [in] terrainType int terrain type
         *  @throw std::invalid_argument if humidity is NaN or infinite.
         */
        Environment(float humidity, int temperature, int altitude, int terrainType);
        /**
         *  @brief loads a JsonBox::Object
         *  @param [in] o JsonBox::Object
         *  @throw std::invalid_argument if humidity is present and NaN/infinite.
         */
        explicit Environment(JsonBox::Object o);
        /**
         *  @brief the fields are public, so a value that was valid at
         *  construction can be changed afterwards; the code that consumes
         *  an environment calls this again.
         *  @throw std::invalid_argument if humidity is NaN or infinite.
         */
        void validate() const;
        /**
         *  @brief converts to JsonBox::Value
         *  @return JsonBox::Value
         */
        JsonBox::Value toJson() const noexcept;
        // data
        float humidity;
        int temperature;
        int altitude;
        int terrainType;
    };
} // namespace fe

#endif // FLOWER_EVOLVER_ENVIRONMENT_HPP
