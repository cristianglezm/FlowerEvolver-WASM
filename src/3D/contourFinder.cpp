#include <3D/contourFinder.hpp>
#include <vector>
#include <array>
#include <set>

namespace fe{
    // Helper function to check alpha at a given coordinate
    inline bool isOpaque(const std::vector<std::uint8_t>& imageData, int x, int y, int width, int height, int alphaThreshold){
        if(x < 0 || x >= width || y < 0 || y >= height){
            return false;
        }
        // Index for Alpha channel (RGBA layout)
        std::size_t index = (static_cast<std::size_t>(y) * width + x) * 4 + 3;
        return imageData[index] >= alphaThreshold;
    }
    bool findContourMoore(
        const std::vector<std::uint8_t>& imageData,
        int width,
        int height,
        int alphaThreshold,
        std::vector<fe::Vec2i>& contourPoints)
    {
        contourPoints.clear();
        if(width <= 0 || height <= 0 || imageData.size() < static_cast<std::size_t>(width * height * 4)){
            return false;
        }
        auto findStartPoint = [](
            const std::vector<std::uint8_t>& imageData,
            int width,
            int height,
            int alphaThreshold) -> fe::Vec2i{
                fe::Vec2i startPoint(-1, -1);
                for(int y = 0; y < height; ++y){
                    for(int x = 0; x < width; ++x){
                        if(isOpaque(imageData, x, y, width, height, alphaThreshold)){
                            return fe::Vec2i(x, y);
                        }
                    }
                }
                return startPoint;
            };
        fe::Vec2i startPoint = findStartPoint(imageData, width, height, alphaThreshold);
        if(startPoint.x == -1){
            return false;
        }
        contourPoints.emplace_back(startPoint);    
        // Define Moore neighbors (8-connectivity) in clockwise order starting from East
        // Indices:   0       1       2       3       4       5       6       7
        // Offsets: (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1,-1), (0,-1), (1,-1)
        //          E      SE     S      SW     W      NW     N      NE
        const std::array<fe::Vec2i, 8> neighbors = {
            {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}}
        };
        fe::Vec2i currentPoint = startPoint;
        // Start by checking the neighbor to the East of the start point initially.
        // This simulates having come from the pixel to the East.
        int initialNeighborIndex = 4;
        int currentNeighborIndex = initialNeighborIndex;
        do{
            // Search neighbors clockwise starting from the one *after* the direction we came from
            // Start check from next neighbor clockwise
            int searchStartIndex = (currentNeighborIndex + 1) % 8;
            fe::Vec2i nextPoint = {-1, -1};
            int foundNeighborIndex = -1;
            for(int i = 0; i < 8; ++i){
                int neighborIndex = (searchStartIndex + i) % 8;
                fe::Vec2i neighborOffset = neighbors[neighborIndex];
                fe::Vec2i checkPos = currentPoint + neighborOffset;
                if(isOpaque(imageData, checkPos.x, checkPos.y, width, height, alphaThreshold)){
                    nextPoint = checkPos;
                    foundNeighborIndex = neighborIndex;
                    break;
                }
            }
            if(nextPoint.x == -1){
                 // This shouldn't happen if startPoint was valid unless it's an isolated pixel
                 // or only has diagonal connections in a specific setup.
                 // If it happens, the contour is likely trivial (1 point).
                 if(contourPoints.size() == 1 && startPoint == currentPoint){
                     // If stuck at start, it's likely isolated, return false
                     contourPoints.clear();
                     return false;
                 }
                  // Otherwise might be end of a line, break trace?
                  break;
            }
            // Opposite index is (foundNeighborIndex + 4) % 8.
            currentNeighborIndex = (foundNeighborIndex + 4) % 8;
            // Move to the next point
            currentPoint = nextPoint;
            // Add to contour if it's not the starting point (to avoid duplication)
            if(currentPoint != startPoint) {
                contourPoints.emplace_back(currentPoint);
            }else if(contourPoints.size() > 1){
                 // If we returned to start AND we've added other points, we are done.
                 break;
            }
            // Safety break to prevent infinite loops in unexpected cases
           if(contourPoints.size() > static_cast<size_t>(width * height * 2)){
               // Contour is unreasonably long, likely an error
               contourPoints.clear();
               return false;
           }
        }while(currentPoint != startPoint || contourPoints.size() <= 1);
        // Continue as long as we are not back at the start point,
        // OR if we are back at the start point but haven't moved anywhere else yet.
        // Need at least 3 points for simplification/geometry generation
        return contourPoints.size() >= 3;
    }
} // namespace fe
