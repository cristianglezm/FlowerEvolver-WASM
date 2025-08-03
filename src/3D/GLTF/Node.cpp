#include <3D/GLTF/Node.hpp>

namespace fe::gltf{
        Node::Node() noexcept
        : name{std::nullopt}
        , children{std::nullopt}
        , mesh{std::nullopt}{}
        Node Node::makeGroup(const std::string& name, const std::vector<int>& children) noexcept{
            auto group = Node{};
            group.name = name;
            std::vector<double> indices{};
            indices.reserve(children.size());
            for(const auto& nodeIndex : children){
                indices.emplace_back(static_cast<double>(nodeIndex));
            }
            group.children = indices;
            return group;
        }
        Node Node::makeNode(const std::string& name, const Mesh& mesh) noexcept{
            auto meshNode = Node{};
            meshNode.name = name;
            meshNode.mesh = static_cast<double>(mesh.index);
            return meshNode;
        }
} // namespace fe::gltf