#include <3D/utils.hpp>
#include <cmath>
#include <limits>
#include <algorithm>

namespace fe{
    float pointLineSegmentDistanceSq(const fe::Vec2i& p, const fe::Vec2i& a, const fe::Vec2i& b){
        fe::Vec2i ab(b.x - a.x, b.y - a.y);
        fe::Vec2i ap(p.x - a.x, p.y - a.y);
        int lenSq = ab.x * ab.x + ab.y * ab.y;
        if(lenSq == 0){
            return static_cast<float>(ap.x * ap.x + ap.y * ap.y);
        }
        float t = static_cast<float>(dot(ap, ab)) / lenSq;
        t = std::clamp(t, 0.0f, 1.0f);
        // Calculate the closest point on the line segment to p
        fe::Vec2f closestPoint(static_cast<float>(a.x) + t * ab.x,
                        static_cast<float>(a.y) + t * ab.y);
        // Calculate the vector from the closest point to p
        fe::Vec2f diff(static_cast<float>(p.x) - closestPoint.x,
                static_cast<float>(p.y) - closestPoint.y);
        return diff.x * diff.x + diff.y * diff.y;
    }
} // namespace fe