#include <3D/contourSimplifier.hpp>
#include <3D/utils.hpp>
#include <vector>
#include <cmath>

namespace fe{
    void douglasPeuckerRecursive(const std::vector<Vec2i>& points, 
            std::size_t firstIndex, std::size_t lastIndex, 
            float epsilonSq, std::vector<Vec2i>& result){
        float maxDistSq = 0.0f;
        std::size_t indexWithMaxDist = firstIndex;
        for(std::size_t i=firstIndex+1; i<lastIndex;++i){
            float distSq = pointLineSegmentDistanceSq(points[i], points[firstIndex], points[lastIndex]);
            if(distSq > maxDistSq){
                maxDistSq = distSq;
                indexWithMaxDist = i;
            }
        }
        if(maxDistSq > epsilonSq){
            // Recursive call for the first part of the curve (before the max distance point)
            if(indexWithMaxDist > firstIndex + 1){
                douglasPeuckerRecursive(points, firstIndex, indexWithMaxDist, epsilonSq, result);
            }else{
                // If the segment is just two points, the furthest point is one of them, add it directly
                // (It should already be added by the previous level's 'firstIndex' or the subsequent 'lastIndex')
                // Add the intermediate point
                result.emplace_back(points[indexWithMaxDist]);
            }
            // Add the point that caused the split (the one with max distance)
            // This point is guaranteed to be kept in the simplified polyline
            result.emplace_back(points[indexWithMaxDist]);
            // Recursive call for the second part of the curve (after the max distance point)
            // Need at least 3 points (indexWithMaxDist, intermediate, lastIndex)
            if(lastIndex > indexWithMaxDist + 1){
                douglasPeuckerRecursive(points, indexWithMaxDist, lastIndex, epsilonSq, result);
            }else{
                // Segment is just two points, handled by adding the end point 'lastIndex' later.
                // Add the end point if it's directly after indexWithMaxDist
                result.emplace_back(points[lastIndex]);
            }
        }
        // If max distance is not greater than epsilonSq, all intermediate points between
        // firstIndex and lastIndex can be discarded. The final segment [firstIndex, lastIndex]
        // will be added when the recursion unwinds and adds the 'lastIndex' point.
    }
    void simplifyContour(const std::vector<Vec2i>& points, float epsilon, std::vector<Vec2i>& result){
        result.clear();
        if(points.size() < 3 || epsilon < 0.0f){
            result = points;
            return;
        }
        float epsilonSq = epsilon * epsilon;
        result.emplace_back(points.front());
        douglasPeuckerRecursive(points, 0, points.size() - 1, epsilonSq, result);
        result.emplace_back(points.back());
        // Check if the first and last points of the result are the same
        // If so, and if the input was likely closed, remove the duplicate last point.
        if(result.size() > 1 && result.front().x == result.back().x && 
            result.front().y == result.back().y){
            // Check if input likely closed (heuristic: start and end points are close)
            fe::Vec2i diff = points.front() - points.back();
            if(fe::dot(diff, diff) < 4){
                result.pop_back();
            }
        }
    }
    // This version assumes result is already populated with the start point.
    void douglasPeucker(const std::vector<Vec2i>& points, float epsilonSq, std::vector<Vec2i>& result){
        if(points.size() < 3) return;
        douglasPeuckerRecursive(points, 0, points.size() - 1, epsilonSq, result);
    }
} // namespace fe