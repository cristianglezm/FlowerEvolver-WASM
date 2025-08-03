#ifndef FLOWER_EVOLVER_3D_GLTF_JSON_WRITER_HPP
#define FLOWER_EVOLVER_3D_GLTF_JSON_WRITER_HPP

#include <3D/GLTF/Scene.hpp>
#include <3D/FlowerParameters.hpp>
#include <string>
#include <sstream>
#include <JsonBox.h>

namespace fe::gltf{
    JsonBox::Array toJsonArray(const fe::Vec3f& vec);
    JsonBox::Array toJsonArray(const fe::Vec4f& vec);
    /**
     * @brief converts a fe::gltf::Scene to a glTF 2.0 JSON from the provided scene.
     * @details Serializes the scene's mesh parts, materials, and textures into
     *          a glTF asset with embedded Base64 binary data for geometry and images.
     *
     * @param scene const fe::gltf::Scene& The populated fe::gltf::Scene object.
     * @param params const fe::FlowerParameters& The FlowerParameters used for generation (can be used for glTF extras).
     * @return A JsonBox::Value containing the glTF JSON.
     */
    JsonBox::Value toJson(const fe::gltf::Scene& scene, const fe::FlowerParameters& params);
    /**
     * @brief converts a JsonBox::Value to a std::string.
     * @param json const JsonBox::Value&
     * @return A std::string containing the glTF stringified json.
     */
    std::string toJsonStr(const JsonBox::Value& json);
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_JSON_WRITER_HPP
