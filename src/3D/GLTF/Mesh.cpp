#include <3D/GLTF/Mesh.hpp>
#include <3D/GLTF/Vertex.hpp>

#include <algorithm>

namespace fe::gltf{
    Mesh::Mesh() noexcept
    : name{""}
    , vertices{}
    , indices{}
    , materialIndex{-1}
    , index{-1}
    , minBounds{std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()}
    , maxBounds{std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()} 
    , hasNormals{false}
    , hasTexCoords0{false}{}
    Mesh::Mesh(int meshIndex, const std::string& name) noexcept
    : name{name}
    , vertices{}
    , indices{}
    , materialIndex{-1}
    , index{meshIndex}
    , minBounds{std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max()}
    , maxBounds{std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest(),
                std::numeric_limits<float>::lowest()}
    , hasNormals{false}
    , hasTexCoords0{false}{}
    unsigned int Mesh::addVertex(const Vertex& vertex){
        vertices.push_back(vertex);
        unsigned int vecIndex = static_cast<unsigned int>(vertices.size() - 1);
        minBounds.x = std::min(minBounds.x, vertex.position.x);
        minBounds.y = std::min(minBounds.y, vertex.position.y);
        minBounds.z = std::min(minBounds.z, vertex.position.z);
        maxBounds.x = std::max(maxBounds.x, vertex.position.x);
        maxBounds.y = std::max(maxBounds.y, vertex.position.y);
        maxBounds.z = std::max(maxBounds.z, vertex.position.z);
        if(vertex.normal){
            hasNormals = true;
        }
        if(vertex.texCoord0){
            hasTexCoords0 = true;
        }
        return vecIndex;
    }
    void Mesh::addTriangle(unsigned int idx0, unsigned int idx1, unsigned int idx2){
        indices.push_back(idx0);
        indices.push_back(idx1);
        indices.push_back(idx2);
    }
}