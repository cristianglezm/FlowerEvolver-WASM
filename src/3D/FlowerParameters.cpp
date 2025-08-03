#include <3D/FlowerParameters.hpp>
#include <Stats.hpp>

namespace fe{
    FlowerParameters::FlowerParameters() noexcept
    : sex{Stats::Sex::Both}
    , useNormals{true}
    , useEmissive{false}
    , stemHeight{0.5f}
    , stemRadius{0.005f}
    , stemSegments{12}
    , pistilStyleHeight{0.050f}
    , pistilStyleRadius{0.00050f}
    , pistilStigmaRadialSegments{12}
    , pistilStigmaHeight{0.002f}
    , pistilStigmaMaxWidthFactor{2.0f}
    , pistilStigmaTipNarrowFactor{0.01}
    , stamenCount{6}
    , stamenFilamentRadialSegments{12}
    , stamenFilamentHeight{0.052f}
    , stamenFilamentRadius{0.0005f}
    , stamenAntherRadialSegments{12}
    , stamenAntherHeight{0.002f}
    , petalScaleFactor{0.008f}
    , connectionRadiusPx{0.2f}
    , droopStartRadiusPx{0.4f}
    , peakHeightOffset{0.04f}
    , petalDroopFactor{0.30f}
    , layerVerticalSpacing{0.004f}
    , connectionVerticalOffset{-0.0469f}
    , contourSimplificationTolerance{0.5f}
    , alphaThreshold{10}{}
    FlowerParameters::FlowerParameters(JsonBox::Object o)
    : sex{o["sex"].tryGetInteger(Stats::Sex::Both)}
    , useNormals{o["useNormals"].tryGetBoolean(true)}
    , useEmissive{o["useEmissive"].tryGetBoolean(false)}
    , stemHeight{o["stemHeight"].tryGetFloat(0.5f)}
    , stemRadius{o["stemRadius"].tryGetFloat(0.005f)}
    , stemSegments{o["stemSegments"].tryGetInteger(12)}
    , pistilStyleHeight{o["pistilStyleHeight"].tryGetFloat(0.050f)}
    , pistilStyleRadius{o["pistilStyleRadius"].tryGetFloat(0.00050f)}
    , pistilStigmaRadialSegments{o["pistilStigmaRadialSegments"].tryGetInteger(12)}
    , pistilStigmaHeight{o["pistilStigmaHeight"].tryGetFloat(0.002f)}
    , pistilStigmaMaxWidthFactor{o["pistilStigmaMaxWidthFactor"].tryGetFloat(2.0f)}
    , pistilStigmaTipNarrowFactor{o["pistilStigmaTipNarrowFactor"].tryGetFloat(0.01f)}
    , stamenCount{o["stamenCount"].tryGetInteger(6)}
    , stamenFilamentRadialSegments{o["stamenFilamentRadialSegments"].tryGetInteger(12)}
    , stamenFilamentHeight{o["stamenFilamentHeight"].tryGetFloat(0.052f)}
    , stamenFilamentRadius{o["stamenFilamentRadius"].tryGetFloat(0.0005f)}
    , stamenAntherRadialSegments{o["stamenAntherRadialSegments"].tryGetInteger(12)}
    , stamenAntherHeight{o["stamenAntherHeight"].tryGetFloat(0.002f)}
    , petalScaleFactor{o["petalScaleFactor"].tryGetFloat(0.008f)}
    , connectionRadiusPx{o["connectionRadiusPx"].tryGetFloat(0.2f)}
    , droopStartRadiusPx{o["droopStartRadiusPx"].tryGetFloat(0.4f)}
    , peakHeightOffset{o["peakHeightOffset"].tryGetFloat(0.04f)}
    , petalDroopFactor{o["petalDroopFactor"].tryGetFloat(0.30f)}
    , layerVerticalSpacing{o["layerVerticalSpacing"].tryGetFloat(0.004f)}
    , connectionVerticalOffset{o["connectionVerticalOffset"].tryGetFloat(-0.0469f)}
    , contourSimplificationTolerance{o["contourSimplificationTolerance"].tryGetFloat(0.5f)}
    , alphaThreshold{o["alphaThreshold"].tryGetInteger(10)}{}
    JsonBox::Object FlowerParameters::toJson() const noexcept{
        JsonBox::Object o;
        o["sex"]                            = sex;
        o["useNormals"]                     = useNormals;
        o["useEmissive"]                    = useEmissive;
        o["stemHeight"]                     = stemHeight;
        o["stemRadius"]                     = stemRadius;
        o["stemSegments"]                   = stemSegments;
        o["pistilStyleHeight"]              = pistilStyleHeight;
        o["pistilStyleRadius"]              = pistilStyleRadius;
        o["pistilStigmaRadialSegments"]     = pistilStigmaRadialSegments;
        o["pistilStigmaHeight"]             = pistilStigmaHeight;
        o["pistilStigmaMaxWidthFactor"]     = pistilStigmaMaxWidthFactor;
        o["pistilStigmaTipNarrowFactor"]    = pistilStigmaTipNarrowFactor;
        o["stamenCount"]                    = stamenCount;
        o["stamenFilamentRadialSegments"]   = stamenFilamentRadialSegments;
        o["stamenFilamentHeight"]           = stamenFilamentHeight;
        o["stamenFilamentRadius"]           = stamenFilamentRadius;
        o["stamenAntherRadialSegments"]     = stamenAntherRadialSegments;
        o["stamenAntherHeight"]             = stamenAntherHeight;
        o["petalScaleFactor"]               = petalScaleFactor;
        o["connectionRadiusPx"]             = connectionRadiusPx;
        o["droopStartRadiusPx"]             = droopStartRadiusPx;
        o["peakHeightOffset"]               = peakHeightOffset;
        o["petalDroopFactor"]               = petalDroopFactor;
        o["layerVerticalSpacing"]           = layerVerticalSpacing;
        o["connectionVerticalOffset"]       = connectionVerticalOffset;
        o["contourSimplificationTolerance"] = contourSimplificationTolerance;
        return o;
    }
} // namespace fe
