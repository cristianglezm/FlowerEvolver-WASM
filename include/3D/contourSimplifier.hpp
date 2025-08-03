#ifndef FLOWER_EVOLVER_3D_CONTOUR_SIMPLIFIER_HPP
#define FLOWER_EVOLVER_3D_CONTOUR_SIMPLIFIER_HPP

#include <vector>
#include <3D/Vec.hpp>

namespace fe{
    /**
     * @brief Simplifies a 2D polyline (contour) using the Douglas-Peucker algorithm.
     *
     * Reduces the number of points in a curve composed of line segments, while
     * keeping the simplified curve within a specified distance (epsilon) of the original.
     *
     * @param points Input vector of 2D integer points representing the original contour.
     * @param epsilonSq The square of the maximum distance (tolerance) allowed between the
     *                  original curve and the simplified curve. Using squared distance
     *                  avoids costly square root operations during comparisons.
     * @param result Output vector where the simplified points will be stored. The vector
     *               will typically include the first and last points of the input segment.
     *               The vector is NOT cleared by this function; points are appended.
     */
    void douglasPeucker(const std::vector<fe::Vec2i>& points, float epsilonSq, std::vector<fe::Vec2i>& result);
    /**
     * @brief Overload for douglasPeucker that handles the initial call and setup.
     *
     * This version takes the full contour, adds the start and end points, and calls
     * the recursive helper function. Assumes a closed contour, but works on open ones too.
     *
     * @param points Input vector of 2D integer points representing the original contour.
     * @param epsilon The maximum distance (tolerance) allowed. Will be squared internally.
     * @param result Output vector where the simplified points will be stored. Cleared first.
     */
    void simplifyContour(const std::vector<fe::Vec2i>& points, float epsilon, std::vector<fe::Vec2i>& result);
} // namespace fe

#endif // FLOWER_EVOLVER_3D_CONTOUR_SIMPLIFIER_HPP