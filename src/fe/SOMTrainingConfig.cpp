#include <fe/SOMTrainingConfig.hpp>

#include <cmath>
#include <stdexcept>
#include <string>

namespace{
    // a negative int cast to size_t would wrap to a huge value
    std::size_t positiveSizeOrThrow(int value, const char* fieldName){
        if(value <= 0){
            throw std::invalid_argument(std::string("fe::SOMTrainingConfig - ") + fieldName + " must be > 0");
        }
        return static_cast<std::size_t>(value);
    }
}

namespace fe{
    SOMTrainingConfig::SOMTrainingConfig(std::size_t width, std::size_t height, std::size_t epochs, double baseLR, double baseRadius)
    : width(width)
    , height(height)
    , epochs(epochs)
    , baseLR(baseLR)
    , baseRadius(baseRadius){
        validate();
    }
    SOMTrainingConfig::SOMTrainingConfig(JsonBox::Object o)
    : width(positiveSizeOrThrow(o["width"].tryGetInteger(5), "width"))
    , height(positiveSizeOrThrow(o["height"].tryGetInteger(5), "height"))
    , epochs(positiveSizeOrThrow(o["epochs"].tryGetInteger(100), "epochs"))
    , baseLR(o["baseLR"].tryGetFloat(0.5f))
    , baseRadius(o["baseRadius"].tryGetFloat(2.5f)){
        validate();
    }
    void SOMTrainingConfig::validate() const{
        auto fail = [](const char* field, const char* what){
            throw std::invalid_argument(std::string("fe::SOMTrainingConfig - ") + field + what);
        };
        if(width == 0){ fail("width", " must be > 0"); }
        if(height == 0){ fail("height", " must be > 0"); }
        if(epochs == 0){ fail("epochs", " must be > 0"); }
        // !(x > 0) so NaN is rejected as well
        if(!(baseLR > 0.0) || !std::isfinite(baseLR)){ fail("baseLR", " must be finite and > 0"); }
        if(!(baseRadius > 0.0) || !std::isfinite(baseRadius)){ fail("baseRadius", " must be finite and > 0"); }
    }
    JsonBox::Value SOMTrainingConfig::toJson() const noexcept{
        JsonBox::Object o;
        o["width"] = static_cast<int>(width);
        o["height"] = static_cast<int>(height);
        o["epochs"] = static_cast<int>(epochs);
        o["baseLR"] = baseLR;
        o["baseRadius"] = baseRadius;
        return JsonBox::Value(o);
    }
} // namespace fe
