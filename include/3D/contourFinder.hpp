#ifndef FLOWER_EVOLVER_3D_CONTOUR_FINDER_HPP
#define FLOWER_EVOLVER_3D_CONTOUR_FINDER_HPP

#include <vector>
#include <3D/Vec.hpp>
#include <cstdint>

namespace fe{
    /**
     * @brief Finds the outer boundary contour of the first encountered opaque component
     *        using the Moore-Neighbor tracing algorithm.
     *
     * Scans the image to find a starting opaque pixel on a boundary (adjacent to a
     * transparent pixel). Then, traces the boundary clockwise until returning to the start.
     * Only finds one contour.
     *
     * @param imageData Raw RGBA pixel data (row-major).
     * @param width Image width in pixels.
     * @param height Image height in pixels.
     * @param alphaThreshold Alpha value (0-255) above which a pixel is considered opaque.
     * @param contourPoints Output vector where the found contour points (fe::Vec2i) will be stored in clockwise order. The vector is cleared first.
     * @return true if a contour with at least 3 points was found, false otherwise.
     */
    bool findContourMoore(
        const std::vector<std::uint8_t>& imageData,
        int width,
        int height,
        int alphaThreshold,
        std::vector<fe::Vec2i>& contourPoints
    );
} // namespace fe

#endif // FLOWER_EVOLVER_3D_CONTOUR_FINDER_HPP
