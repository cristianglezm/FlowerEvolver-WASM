#include <3D/GLTF/Scene.hpp>

namespace fe::gltf{
    Scene::Scene(const std::string& id)
    : modelId(id){}
    int Scene::addTexture(const TextureInfo& textureInfo){
        textures.push_back(textureInfo);
        return static_cast<int>(textures.size() - 1);
    }
    int Scene::addMaterial(const Material& material){
        materials.push_back(material);
        return static_cast<int>(materials.size() - 1);
    }
    int Scene::addNode(const Node& node){
        nodes.push_back(node);
        return static_cast<int>(nodes.size() - 1);
    }
    Mesh& Scene::addMeshPart(const Mesh& part){
        meshParts.push_back(part);
        return meshParts.back();
    }
    Mesh& Scene::createMeshPart(const std::string& name){
        meshParts.emplace_back(meshParts.size(), name);
        return meshParts.back();
    }
}
