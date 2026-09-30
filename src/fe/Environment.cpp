#include <fe/Environment.hpp>

#include <cmath>
#include <stdexcept>

namespace fe{
    Environment::Environment() noexcept
    : humidity(0.5f)
    , temperature(20)
    , altitude(0)
    , terrainType(0){}
    Environment::Environment(float humidity, int temperature, int altitude, int terrainType)
    : humidity(humidity)
    , temperature(temperature)
    , altitude(altitude)
    , terrainType(terrainType){
        validate();
    }
    Environment::Environment(JsonBox::Object o)
    : humidity(o["humidity"].tryGetFloat(0.5f))
    , temperature(o["temperature"].tryGetInteger(20))
    , altitude(o["altitude"].tryGetInteger(0))
    , terrainType(o["terrainType"].tryGetInteger(0)){
        validate();
    }
    void Environment::validate() const{
        if(!std::isfinite(humidity)){
            throw std::invalid_argument("fe::Environment - humidity must be finite (not NaN/Inf)");
        }
    }
    JsonBox::Value Environment::toJson() const noexcept{
        JsonBox::Object o;
        o["humidity"] = JsonBox::Value(humidity);
        o["temperature"] = JsonBox::Value(temperature);
        o["altitude"] = JsonBox::Value(altitude);
        o["terrainType"] = JsonBox::Value(terrainType);
        return JsonBox::Value(o);
    }
} // namespace fe
