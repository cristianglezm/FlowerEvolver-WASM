#ifndef FLOWER_EVOLVER_3D_GLTF_SCENE_HPP
#define FLOWER_EVOLVER_3D_GLTF_SCENE_HPP

#include <string>
#include <vector>
#include <optional>
#include <limits>

#include <3D/Vec.hpp>
#include <3D/GLTF/TextureInfo.hpp>
#include <3D/GLTF/Material.hpp>
#include <3D/GLTF/Mesh.hpp>
#include <3D/GLTF/Node.hpp>

namespace fe::gltf{
    /**
     * @brief Top-level data structure for a glTF scene, typically representing a flower model.
     */
    struct Scene final{
        /**
         * @brief Constructor.
         * @param id The model identifier.
         */
        explicit Scene(const std::string& id);
        /**
         * @brief Adds a texture to the scene's texture list.
         * @param textureInfo The fe::gltf::textureInfo object describing the texture.
         * @return The 0-based index of the added texture in the `textures` vector.
         */
        int addTexture(const TextureInfo& textureInfo);
        /**
         * @brief Adds a material to the scene's material list.
         * @param material The fe::gltf::Material object describing the material.
         * @return The 0-based index of the added material in the `materials` vector.
         */
        int addMaterial(const Material& material);
        /**
         * @brief Adds a mesh part to the scene.
         * @warning do not add or create another mesh until you are done with the current one.
         * @param part The fe::gltf::Mesh to add.
         * @return A reference to the added GltfMeshPart within the `meshParts` vector.
         *         This can be useful if further modifications are needed after adding.
         */
        Mesh& addMeshPart(const Mesh& part);
        /**
         * @brief Adds a node to the scene.
         * @param node const Node&
         * @return The 0-based index of the added node in the `nodes` vector.
         */
        int addNode(const Node& node);
        /**
         * @brief Creates and adds a new mesh part to the scene, returning a reference to it.
         * @warning do not add or create another mesh until you are done with the current one.
         * @param name The name for the new mesh part.
         * @return A reference to the newly created Mesh.
         */
        Mesh& createMeshPart(const std::string& name);
        // data
        std::string modelId;
        /** @brief Collection of distinct geometric parts that make up the model.
         *         Each Mesh will be exported as a separate glTF `mesh` and `node`.
         */
        std::vector<Mesh> meshParts;
        /** @brief Collection of textures used by materials in this scene. */
        std::vector<TextureInfo> textures;
        /** @brief Collection of materials used by mesh parts in this scene. */
        std::vector<Material> materials;
        /** @brief  Collection of nodes to group the meshes */
        std::vector<Node> nodes;
    };
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_SCENE_HPP