#ifndef FLOWER_EVOLVER_3D_GLTF_MESH_HPP
#define FLOWER_EVOLVER_3D_GLTF_MESH_HPP

#include <3D/Vec.hpp>
#include <3D/GLTF/Vertex.hpp>
#include <string>
#include <vector>
#include <optional>
#include <limits>

#include <JsonBox.h>

namespace fe::gltf{
    /**
     * @brief Represents a distinct geometric part of the model.
     *
     * It contains its own vertices, indices, and a reference to a material.
     * Internally, this will correspond to a glTF `mesh` with a single glTF `primitive`.
     */
    struct Mesh final{
        /**
         * @brief default constructor
         */
        Mesh() noexcept;
        /**
         * @brief constructor
         * @param meshIndex int index inside the scene
         * @param name const std::string& name for the mesh
         */
        Mesh(int meshIndex, const std::string& name) noexcept;
        /**
         * @brief Adds a vertex to this mesh part and updates bounds.
         * @param vertex The fe::gltf::Vertex to add.
         * @return The 0-based index of the added vertex within this part's `vertices` vector.
         */
        unsigned int addVertex(const Vertex& vertex);
        /**
         * @brief Adds a triangle to this mesh part.
         * Assumes vertices are already added via `addVertex` and indices are relative to this part.
         * @param idx0 Index of the first vertex of the triangle.
         * @param idx1 Index of the second vertex of the triangle.
         * @param idx2 Index of the third vertex of the triangle.
         */
        void addTriangle(unsigned int idx0, unsigned int idx1, unsigned int idx2);
        // data
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        int materialIndex = -1;
        int index = -1;
        fe::Vec3f minBounds = {
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max()
        };
        fe::Vec3f maxBounds = {
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest()
        };
        bool hasNormals = false;
        bool hasTexCoords0 = false;
    };
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_MESH_HPP