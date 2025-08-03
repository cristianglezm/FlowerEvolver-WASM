#include <3D/GLTF/Material.hpp>

namespace fe::gltf{
    Material Material::createStemMaterial(){
        Material stemMat{};
        stemMat.name = "Stem";
        stemMat.baseColorFactor = {0.2f, 0.6f, 0.2f, 1.0f};
        stemMat.metallicFactor = 0.1f;
        stemMat.roughnessFactor = 0.9f;
        stemMat.doubleSided = true;
        return stemMat;
    }
    Material Material::createPistilStyleMaterial(){
        Material PistilStyle{};
        PistilStyle.name = "PistilStyle";
        PistilStyle.baseColorFactor = {0.2f, 0.6f, 0.2f, 1.0f};
        PistilStyle.metallicFactor = 0.1f;
        PistilStyle.roughnessFactor = 0.9f;
        PistilStyle.doubleSided = true;
        return PistilStyle;
    }
    Material Material::createPistilStigmaMaterial(int normal_tex_idx){
        Material PistilStigma{};
        PistilStigma.name = "PistilStigma";
        PistilStigma.baseColorFactor = {0.2f, 0.8f, 0.2f, 1.0f};
        PistilStigma.normalTextureIndex = normal_tex_idx;
        PistilStigma.metallicFactor = 0.0f;
        PistilStigma.roughnessFactor = 1.0f;
        PistilStigma.doubleSided = true;
        return PistilStigma;
    }
    Material Material::createStamenFilamentMaterial(){
        Material StamenFilament{};
        StamenFilament.name = "StamenFilament";
        StamenFilament.baseColorFactor = {0.2f, 0.6f, 0.2f, 1.0f};
        StamenFilament.metallicFactor = 0.1f;
        StamenFilament.roughnessFactor = 0.9f;
        StamenFilament.doubleSided = true;
        return StamenFilament;
    }
    Material Material::createStamenAntherMaterial(int normal_tex_idx){
        Material StamenAnther{};
        StamenAnther.name = "StamenAnther";
        StamenAnther.baseColorFactor = {0.8f, 0.8f, 0.0f, 1.0f};
        StamenAnther.normalTextureIndex = normal_tex_idx;
        StamenAnther.metallicFactor = 0.1f;
        StamenAnther.roughnessFactor = 0.9f;
        StamenAnther.doubleSided = true;
        return StamenAnther;
    }
    Material Material::createPetalMaterial(const std::string& petal_name, int texture_idx, int normal_idx, int emissive_idx){
        Material petalMat{};
        petalMat.name = petal_name;
        petalMat.baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f};
        if(texture_idx >= 0){
            petalMat.baseColorTextureIndex = texture_idx;
        }
        if(normal_idx >= 0){
            petalMat.normalTextureIndex = normal_idx;
        }
        if(emissive_idx >= 0){
            petalMat.emissiveTextureIndex = emissive_idx;
            petalMat.emissiveFactor = fe::Vec3f(1.0f, 1.0f, 1.0f);
            petalMat.khrMaterialsEmissiveStrength = KHR_MaterialsEmissiveStrength{
                .emissiveStrength = 1.0f
            };
        }
        petalMat.metallicFactor = 0.0f;
        petalMat.roughnessFactor = 0.8f;
        petalMat.doubleSided = true;
        petalMat.alphaMode = "MASK";
        petalMat.alphaCutoff = 0.5f;
        petalMat.khrMaterialsIOR = KHR_MaterialsIOR{
            .ior = 1.4f
        };
        petalMat.khrMaterialsTransmission = KHR_MaterialsTransmission{
            .transmissionFactor = 0.09f, 
            .transmissionTextureIndex = std::nullopt
        };
        return petalMat;
    }
}
