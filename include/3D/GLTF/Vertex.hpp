#ifndef FLOWER_EVOLVER_3D_GLTF_VERTEX_HPP
#define FLOWER_EVOLVER_3D_GLTF_VERTEX_HPP

#include <3D/Vec.hpp>

#include <optional>

namespace fe::gltf{
    /**
     * @brief Represents a single vertex with all its geometric attributes for glTF.
     */
    struct Vertex{
        /**
         * @brief Default constructor.
         */
        Vertex() = default;
        /**
         * @brief Constructor with position.
         * @param pos The vertex position.
         */
        explicit Vertex(const fe::Vec3f& pos);
        /**
         * @brief Constructor with all common attributes.
         * @param pos The vertex position.
         * @param norm The vertex normal.
         * @param uv0 The vertex texture coordinate (TEXCOORD_0).
         */
        Vertex(const fe::Vec3f& pos, const fe::Vec3f& norm, const fe::Vec2f& uv0);
        /**
         * @brief Constructor with all common attributes.
         * @param pos The vertex position.
         * @param norm The vertex normal.
         * @param uv0 The vertex texture coordinate (TEXCOORD_0).
         */
        Vertex(const fe::Vec3f& pos, const std::optional<fe::Vec3f>& norm, const std::optional<fe::Vec2f>& uv0);
        // data
        fe::Vec3f position;
        std::optional<fe::Vec3f> normal;
        std::optional<fe::Vec2f> texCoord0;
    };
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_VERTEX_HPP