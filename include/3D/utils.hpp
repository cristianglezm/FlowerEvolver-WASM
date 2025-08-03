#ifndef FLOWER_EVOLVER_3D_UTILS_HPP
#define FLOWER_EVOLVER_3D_UTILS_HPP

#include <3D/Vec.hpp>

namespace fe{
    /**
     * @brief Dot product for 2D integer vectors
     * @param a const Vec2f&
     * @param b const Vec2f&
     * @return int
     */
    inline int dot(const fe::Vec2i& a, const fe::Vec2i& b){
        return a.x * b.x + a.y * b.y;
    }
    /**
     * @brief Calculate the squared perpendicular distance from a point p to the line segment defined by a and b.
     * @details
     * Calculate the projection of ap onto ab
     * Parameter t = dot(ap, ab) / lengthSq(ab)
     * This indicates where the projection falls relative to the segment
     * t < 0: Projection is before point a
     * t > 1: Projection is after point b
     * 0 <= t <= 1: Projection is within the segment [a, b]
     * @param p const Vec2f&
     * @param a const Vec2f&
     * @param b const Vec2f&
     * @return float
     */
    float pointLineSegmentDistanceSq(const fe::Vec2i& p, const fe::Vec2i& a, const fe::Vec2i& b);
} // namespace fe

#endif // FLOWER_EVOLVER_3D_UTILS_HPP