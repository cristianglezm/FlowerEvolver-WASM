#ifndef FLOWER_EVOLVER_3D_FLOWER_PARAMETERS_HPP
#define FLOWER_EVOLVER_3D_FLOWER_PARAMETERS_HPP

#include <JsonBox.h>

namespace fe{
    /**
     * @brief creation parameters for 3d flowers
     */
    struct FlowerParameters final{
        /**
         * @brief default constructor
         */
        FlowerParameters() noexcept;
        /**
         * @brief deserializable constructor
         * @param o JsonBox::Object
         */
        FlowerParameters(JsonBox::Object o);
        /**
         * @brief serializes the parameters
         * @return JsonBox::Object
         */
        JsonBox::Object toJson() const noexcept;
        // data
        int sex;
        bool useNormals;
        bool useEmissive;
        // Stem
        float stemHeight;
        float stemRadius;
        int stemSegments;
        // Pistil style
        float pistilStyleHeight;
        float pistilStyleRadius;
        // Pistil Stigma
	    int pistilStigmaRadialSegments;
	    float pistilStigmaHeight;
	    float pistilStigmaMaxWidthFactor;
        float pistilStigmaTipNarrowFactor;
        // stamen filament
        int stamenCount;
        int stamenFilamentRadialSegments;
        float stamenFilamentHeight;
        float stamenFilamentRadius;
        // stamen anther
        int stamenAntherRadialSegments;
        float stamenAntherHeight;
        // Petal Scaling & Shape
        float petalScaleFactor;
        float connectionRadiusPx;
        float droopStartRadiusPx;
        float peakHeightOffset;
        // Droop amount per pixel distance beyond droopStartRadiusPx, scaled by petalScaleFactor
        float petalDroopFactor;
        // Stacking & Connection
        float layerVerticalSpacing;
        float connectionVerticalOffset;
        // Increase -> less detail, fewer verts. Try 0.5 to 3.0
        float contourSimplificationTolerance;
        // Alpha value below which pixels are considered transparent
        int alphaThreshold;
    };
} // namespace fe

#endif // FLOWER_EVOLVER_3D_FLOWER_PARAMETERS_HPP
