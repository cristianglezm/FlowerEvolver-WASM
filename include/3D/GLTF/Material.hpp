#ifndef FLOWER_EVOLVER_3D_GLTF_MATERIAL_HPP
#define FLOWER_EVOLVER_3D_GLTF_MATERIAL_HPP

#include <optional>
#include <string>

#include <3D/Vec.hpp>

namespace fe::gltf{
    /**
     * @brief KHR_MaterialsTransmission extension properties.
     */
    struct KHR_MaterialsTransmission final{
        float transmissionFactor = 0.0f;
        std::optional<int> transmissionTextureIndex;
        // If not using TEXCOORD_0
        // std::optional<int> transmissionTextureTexCoord;
    };
    /**
     * @brief KHR_MaterialsVolume extension properties.
     */
    struct KHR_MaterialsVolume final{
        float thicknessFactor = 0.0f;
        std::optional<int> thicknessTextureIndex;
        // std::optional<int> thicknessTextureTexCoord;
        float attenuationDistance = 0.0f;
        fe::Vec3f attenuationColor = {1.0f, 1.0f, 1.0f};
    };
    /**
     * @brief KHR_MaterialsIOR extension properties.
     */
    struct KHR_MaterialsIOR final{
        float ior = 1.0f;
    };
    /**
     * @brief KHR_MaterialsEmissiveStrength extension properties
     */
    struct KHR_MaterialsEmissiveStrength final {
       float emissiveStrength = 1.0f;
    };
    /**
     * @brief KHR_MaterialsClearcoat extension properties.
     */
    struct KHR_MaterialsClearcoat final{
        float clearcoatFactor = 0.0f;
        std::optional<int> clearcoatTextureIndex;
        // std::optional<int> clearcoatTextureTexCoord;
        float clearcoatRoughnessFactor = 0.0f;
        std::optional<int> clearcoatRoughnessTextureIndex;
        // std::optional<int> clearcoatRoughnessTextureTexCoord;
        std::optional<int> clearcoatNormalTextureIndex;
        // std::optional<int> clearcoatNormalTextureTexCoord;
        // std::optional<float> clearcoatNormalTextureScale;
    };
    /**
     * @brief Defines material properties for rendering.
     */
    struct Material final{
        /**
         * @brief Default constructor.
         */
        Material() = default;
        /**
         * @brief Creates a default stem material.
         * @return A Material configured for a stem.
         */
        static Material createStemMaterial();
        /**
         * @brief creates a default Pistil style Material
         * @return A Material configured for a Pistil style.
         */
        static Material createPistilStyleMaterial();
        /**
         * @brief creates a default Pistil stigma material
         * @param normal_tex_idx The index of the normal texture for the Stigma.
         * @return A Material configured for Pistil Stigma.
         */
        static Material createPistilStigmaMaterial(int normal_tex_idx);
        /**
         * @brief creates a default Stamen filament material
         * @return A Material configured for Stamen Filament.
         */
        static Material createStamenFilamentMaterial();
        /**
         * @brief creates a default Stamen anther material
         * @param normal_tex_idx The index of the normal texture for the Anther.
         * @return A Material configured for Stamen Anther.
         */
        static Material createStamenAntherMaterial(int normal_tex_idx);
        /**
         * @brief Creates a default petal material.
         * @param petal_name const std::string& The name for the petal material.
         * @param texture_idx int The index of the base color texture for this petal.
         * @param normal_idx int the index of the normal texture for this petal.
         * @param emissive_idx int the index of the emissive texture for this petal.
         * @return A Material configured for a petal.
         */
        static Material createPetalMaterial(const std::string& petal_name, int texture_idx, int normal_idx, int emissive_idx);
        // data
        std::string name;
        fe::Vec4f baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f};
        std::optional<int> baseColorTextureIndex;
        float metallicFactor = 0.0f;
        float roughnessFactor = 0.5f;
        std::optional<int> metallicRoughnessTextureIndex;
        std::optional<int> normalTextureIndex;
        std::optional<float> normalTextureScale = 1.0f;
        std::optional<int> occlusionTextureIndex;
        std::optional<float> occlusionTextureStrength = 1.0f;
        std::optional<int> emissiveTextureIndex;
        fe::Vec3f emissiveFactor = {0.0f, 0.0f, 0.0f};
        bool doubleSided = false;
        std::string alphaMode = "OPAQUE"; // "OPAQUE", "MASK", "BLEND"
        float alphaCutoff = 0.5f;
        // KHR Extensions
        std::optional<KHR_MaterialsTransmission> khrMaterialsTransmission;
        std::optional<KHR_MaterialsClearcoat> khrMaterialsClearcoat;
        std::optional<KHR_MaterialsVolume> khrMaterialsVolume;
        std::optional<KHR_MaterialsIOR> khrMaterialsIOR;
        std::optional<KHR_MaterialsEmissiveStrength> khrMaterialsEmissiveStrength;
    };
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_MATERIAL_HPP
