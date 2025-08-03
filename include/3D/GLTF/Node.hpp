#ifndef FLOWER_EVOLVER_3D_GLTF_NODE_HPP
#define FLOWER_EVOLVER_3D_GLTF_NODE_HPP

#include <string>
#include <optional>
#include <vector>

#include <3D/GLTF/Mesh.hpp>
#include <JsonBox.h>

namespace fe::gltf{
    /**
     * @brief Node
     */
    struct Node{
        /**
         * @brief default constructor
         */
        Node() noexcept;
        /**
         * @brief it makes a group of nodes
         * @param name const std::string& name for the node group
         * @param children const std::vector<int>&
         * @return Node
         */
        static Node makeGroup(const std::string& name, const std::vector<int>& children) noexcept;
        /**
         * @brief it adds a mesh node
         * @param name const std::string& name for the mesh node
         * @param mesh const fe::gltf::Mesh& mesh to get the index from.
         * @return Node
         */
        static Node makeNode(const std::string& name, const Mesh& mesh) noexcept;
        // data
        std::optional<std::string> name;
        std::optional<std::vector<double>> children;
        std::optional<JsonBox::Object> extra;
        std::optional<double> mesh;
        // Includes only the node fields required for mesh grouping and assignment.
    };
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_NODE_HPP