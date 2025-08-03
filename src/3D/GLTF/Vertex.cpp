#include <3D/GLTF/Vertex.hpp>

namespace fe::gltf{
    Vertex::Vertex(const fe::Vec3f& pos)
    : position(pos){}
    Vertex::Vertex(const fe::Vec3f& pos, const fe::Vec3f& norm, const fe::Vec2f& uv0)
    : position(pos)
    , normal(norm)
    , texCoord0(uv0){}
    Vertex::Vertex(const fe::Vec3f& pos, const std::optional<fe::Vec3f>& norm, const std::optional<fe::Vec2f>& uv0)
    : position(pos)
    , normal(norm)
    , texCoord0(uv0){}
}
