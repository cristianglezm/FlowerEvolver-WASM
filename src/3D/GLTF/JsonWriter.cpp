#include <3D/GLTF/JsonWriter.hpp>

#include <numeric>

namespace fe::gltf{
    // Helper for Vec3f (if not already part of a ToJson system)
    JsonBox::Array toJsonArray(const fe::Vec3f& vec){
        JsonBox::Array arr;
        arr.emplace_back(static_cast<double>(vec.x));
        arr.emplace_back(static_cast<double>(vec.y));
        arr.emplace_back(static_cast<double>(vec.z));
        return arr;
    }
    // Helper for Vec4f
    JsonBox::Array toJsonArray(const fe::Vec4f& vec){
        JsonBox::Array arr;
        arr.emplace_back(static_cast<double>(vec.r));
        arr.emplace_back(static_cast<double>(vec.g));
        arr.emplace_back(static_cast<double>(vec.b));
        arr.emplace_back(static_cast<double>(vec.a));
        return arr;
    }
    JsonBox::Value toJson(const fe::gltf::Scene& scene, const fe::FlowerParameters& params){
        // glTF Constants
        [[maybe_unused]] constexpr int COMPONENT_TYPE_BYTE = 5120;
        [[maybe_unused]] constexpr int COMPONENT_TYPE_UNSIGNED_BYTE = 5121;
        [[maybe_unused]] constexpr int COMPONENT_TYPE_SHORT = 5122;
        [[maybe_unused]] constexpr int COMPONENT_TYPE_UNSIGNED_SHORT = 5123;
        [[maybe_unused]] constexpr int COMPONENT_TYPE_UNSIGNED_INT = 5125;
        [[maybe_unused]] constexpr int COMPONENT_TYPE_FLOAT = 5126;
        [[maybe_unused]] constexpr int TARGET_ARRAY_BUFFER = 34962;
        [[maybe_unused]] constexpr int TARGET_ELEMENT_ARRAY_BUFFER = 34963;
        [[maybe_unused]] constexpr int PRIMITIVE_MODE_TRIANGLES = 4;

        JsonBox::Object gltfRoot;
        JsonBox::Object asset;
        asset["version"] = "2.0";
        asset["generator"] = "Flower Evolver - https://github.com/cristianglezm/FlowerEvolver-WASM";
        JsonBox::Object extras;
        extras["description"] = "Generated 3d flower by FlowerEvolver";
        std::size_t totalVertices = std::accumulate(std::begin(scene.meshParts), std::end(scene.meshParts), 0u,
            [](auto& a, auto& b){
                return a + b.vertices.size();
            }
        );
        std::size_t totalFaces = std::accumulate(std::begin(scene.meshParts), std::end(scene.meshParts), 0u, 
            [](auto& a, auto& b){
                return a + b.indices.size() / 3;
            }
        );
        extras["vertices"] = static_cast<double>(totalVertices);
        extras["textureCoordinates"] = static_cast<double>(totalVertices);
        extras["normals"] = static_cast<double>(totalVertices);
        extras["faces"] = static_cast<double>(totalFaces);
        extras["parameters"] = params.toJson();
        gltfRoot["extras"] = extras;
        gltfRoot["asset"] = asset;
        // Main Binary Buffer
        std::vector<std::uint8_t> binaryBufferData;
        auto appendToBinaryBuffer = [&](const void* data, std::size_t byteLength){
            const auto* bytes = static_cast<const std::uint8_t*>(data);
            binaryBufferData.insert(binaryBufferData.end(), bytes, bytes + byteLength);
        };
        JsonBox::Array gltfScenes;
        JsonBox::Object defaultScene;
        defaultScene["name"] = "Flower_" + scene.modelId + "_Scene";
        // Indices of root nodes in this scene
        JsonBox::Array sceneNodes;
        // Index of the default scene
        gltfRoot["scene"] = 0;
        JsonBox::Array gltfNodes;
        JsonBox::Array gltfMeshes;
        JsonBox::Array gltfAccessors;
        JsonBox::Array gltfBufferViews;
        JsonBox::Array gltfMaterials;
        JsonBox::Array gltfTextures;
        JsonBox::Array gltfImages;
        JsonBox::Array gltfSamplers;
        // Samplers (define one default sampler)
        JsonBox::Object defaultSampler;
        defaultSampler["magFilter"] = 9729; // LINEAR
        defaultSampler["minFilter"] = 9987; // LINEAR_MIPMAP_LINEAR
        defaultSampler["wrapS"] = 10497; // REPEAT
        defaultSampler["wrapT"] = 10497; // REPEAT
        gltfSamplers.emplace_back(defaultSampler);
        // Images and Textures
        for(std::size_t i = 0; i < scene.textures.size(); ++i){
            const fe::gltf::TextureInfo& texInfo = scene.textures[i];
            JsonBox::Object imageJson;
            imageJson["name"] = texInfo.name;
            imageJson["mimeType"] = texInfo.mimeType;
            imageJson["uri"] = texInfo.uri;
            gltfImages.emplace_back(imageJson);
            JsonBox::Object textureJson;
            textureJson["name"] = texInfo.name;
            // Index into gltfImages
            textureJson["source"] = static_cast<double>(i);
            if(!gltfSamplers.empty()){
                // Index into gltfSamplers (using the first one)
                textureJson["sampler"] = 0.0;
            }
            gltfTextures.emplace_back(textureJson);
        }
        // Materials
        for(const auto& matInfo : scene.materials){
            JsonBox::Object materialJson;
            materialJson["name"] = matInfo.name;
            JsonBox::Object pbrMetallicRoughness;
            pbrMetallicRoughness["baseColorFactor"] = toJsonArray(matInfo.baseColorFactor);
            if(matInfo.baseColorTextureIndex){
                JsonBox::Object texRef;
                texRef["index"] = static_cast<double>(*matInfo.baseColorTextureIndex);
                texRef["texCoord"] = 0.0;
                pbrMetallicRoughness["baseColorTexture"] = texRef;
            }
            pbrMetallicRoughness["metallicFactor"] = static_cast<double>(matInfo.metallicFactor);
            pbrMetallicRoughness["roughnessFactor"] = static_cast<double>(matInfo.roughnessFactor);
            if(matInfo.metallicRoughnessTextureIndex){
                JsonBox::Object texRef;
                texRef["index"] = static_cast<double>(*matInfo.metallicRoughnessTextureIndex);
                pbrMetallicRoughness["metallicRoughnessTexture"] = texRef;
            }
            materialJson["pbrMetallicRoughness"] = pbrMetallicRoughness;
            if(matInfo.normalTextureIndex){
                JsonBox::Object texRef;
                texRef["index"] = static_cast<double>(*matInfo.normalTextureIndex);
                texRef["scale"] = 1.0f;
                materialJson["normalTexture"] = texRef;
            }
            // Occlusion (ambient occlusion) texture
            if(matInfo.occlusionTextureIndex){
                JsonBox::Object occlusionJson;
                occlusionJson["index"] = *matInfo.occlusionTextureIndex;
                if(*matInfo.occlusionTextureStrength != 1.0f){
                    occlusionJson["strength"] = *matInfo.occlusionTextureStrength;
                }
                materialJson["occlusionTexture"] = occlusionJson;
            }
            // Emissive texture  
            if(matInfo.emissiveTextureIndex){
                JsonBox::Object emissiveJson;
                emissiveJson["index"] = *matInfo.emissiveTextureIndex;
                materialJson["emissiveTexture"] = emissiveJson;
            }
            if(matInfo.emissiveFactor.x != 0.0f || matInfo.emissiveFactor.y != 0.0f || matInfo.emissiveFactor.z != 0.0f){
                materialJson["emissiveFactor"] = toJsonArray(matInfo.emissiveFactor);
            }
            materialJson["doubleSided"] = matInfo.doubleSided;
            if(matInfo.alphaMode != "OPAQUE"){
                materialJson["alphaMode"] = matInfo.alphaMode;
                if(matInfo.alphaMode == "MASK"){
                    materialJson["alphaCutoff"] = static_cast<double>(matInfo.alphaCutoff);
                }
            }
            // Extensions
            JsonBox::Object extensionsJson;
            bool hasExtensions = false;
            if(matInfo.khrMaterialsTransmission){
                JsonBox::Object transmissionJson;
                transmissionJson["transmissionFactor"] = static_cast<double>(matInfo.khrMaterialsTransmission->transmissionFactor);
                if(matInfo.khrMaterialsTransmission->transmissionTextureIndex){
                    JsonBox::Object texRef;
                    texRef["index"] = static_cast<double>(*matInfo.khrMaterialsTransmission->transmissionTextureIndex);
                    transmissionJson["transmissionTexture"] = texRef;
                }
                extensionsJson["KHR_materials_transmission"] = transmissionJson;
                hasExtensions = true;
            }
            if(matInfo.khrMaterialsVolume){
                JsonBox::Object volumeJson;
                volumeJson["thicknessFactor"] = static_cast<double>(matInfo.khrMaterialsVolume->thicknessFactor);
                 if(matInfo.khrMaterialsVolume->thicknessTextureIndex){
                    JsonBox::Object texRef;
                    texRef["index"] = static_cast<double>(*matInfo.khrMaterialsVolume->thicknessTextureIndex);
                    volumeJson["thicknessTexture"] = texRef;
                 }
                volumeJson["attenuationDistance"] = static_cast<double>(matInfo.khrMaterialsVolume->attenuationDistance);
                volumeJson["attenuationColor"] = toJsonArray(matInfo.khrMaterialsVolume->attenuationColor);
                extensionsJson["KHR_materials_volume"] = volumeJson;
                hasExtensions = true;
            }
            if(matInfo.khrMaterialsClearcoat){
                JsonBox::Object clearcoatJson;
                clearcoatJson["clearcoatFactor"] = static_cast<double>(matInfo.khrMaterialsClearcoat->clearcoatFactor);
                if(matInfo.khrMaterialsClearcoat->clearcoatTextureIndex){
                    JsonBox::Object texRef;
                    texRef["index"] = static_cast<double>(*matInfo.khrMaterialsClearcoat->clearcoatTextureIndex);
                    clearcoatJson["clearcoatTexture"] = texRef;
                }
                clearcoatJson["clearcoatRoughnessFactor"] = static_cast<double>(matInfo.khrMaterialsClearcoat->clearcoatRoughnessFactor);
                if(matInfo.khrMaterialsClearcoat->clearcoatRoughnessTextureIndex){
                    JsonBox::Object texRef;
                    texRef["index"] = static_cast<double>(*matInfo.khrMaterialsClearcoat->clearcoatRoughnessTextureIndex);
                    clearcoatJson["clearcoatRoughnessTexture"] = texRef;
                }
                if(matInfo.khrMaterialsClearcoat->clearcoatNormalTextureIndex){
                    JsonBox::Object texRef;
                    texRef["index"] = static_cast<double>(*matInfo.khrMaterialsClearcoat->clearcoatNormalTextureIndex);
                    clearcoatJson["clearcoatNormalTexture"] = texRef;
                }
                extensionsJson["KHR_materials_clearcoat"] = clearcoatJson;
                hasExtensions = true;
            }
            if(matInfo.khrMaterialsIOR){
                JsonBox::Object ior;
                ior["ior"] = matInfo.khrMaterialsIOR->ior;
                extensionsJson["KHR_materials_ior"] = ior;
                hasExtensions = true;
            }
            if(matInfo.khrMaterialsEmissiveStrength){
                JsonBox::Object emissiveStrength;
                emissiveStrength["emissiveStrength"] = matInfo.khrMaterialsEmissiveStrength->emissiveStrength;
                extensionsJson["KHR_materials_emissive_strength"] = emissiveStrength;
                hasExtensions = true;
            }
            if(hasExtensions){
                materialJson["extensions"] = extensionsJson;
            }
            gltfMaterials.emplace_back(materialJson);
        }
        // Process each mesh
        for(const auto& part : scene.meshParts){
            if(part.vertices.empty() || part.indices.empty()){
                continue;
            }
            JsonBox::Object primitiveJson;
            JsonBox::Object attributesJson;
            // Positions Accessor
            std::size_t positionsByteOffset = binaryBufferData.size();
            for(const fe::gltf::Vertex& v : part.vertices){
                appendToBinaryBuffer(&v.position, sizeof(fe::Vec3f));
            }
            std::size_t positionsByteLength = part.vertices.size() * sizeof(fe::Vec3f);
            JsonBox::Object posBvJson;
            // Single main buffer
            posBvJson["buffer"] = 0.0;
            posBvJson["byteOffset"] = static_cast<double>(positionsByteOffset);
            posBvJson["byteLength"] = static_cast<double>(positionsByteLength);
            posBvJson["target"] = static_cast<double>(TARGET_ARRAY_BUFFER);
            gltfBufferViews.emplace_back(posBvJson);
            int posBvIndex = static_cast<int>(gltfBufferViews.size() - 1);
            JsonBox::Object posAccJson;
            posAccJson["bufferView"] = static_cast<double>(posBvIndex);
            // Relative to bufferView start
            posAccJson["byteOffset"] = 0.0;
            posAccJson["componentType"] = static_cast<double>(COMPONENT_TYPE_FLOAT);
            posAccJson["count"] = static_cast<double>(part.vertices.size());
            posAccJson["type"] = "VEC3";
            // Use pre-calculated bounds
            posAccJson["min"] = toJsonArray(part.minBounds);
            posAccJson["max"] = toJsonArray(part.maxBounds);
            gltfAccessors.emplace_back(posAccJson);
            attributesJson["POSITION"] = static_cast<double>(gltfAccessors.size() - 1);
            // Normals Accessor (if present)
            if(part.hasNormals){
                std::size_t normalsByteOffset = binaryBufferData.size();
                for(const fe::gltf::Vertex& v : part.vertices){
                    // write a default normal if not present.
                    fe::Vec3f normalToWrite = v.normal ? *v.normal : fe::Vec3f{0.0f, 0.0f, 1.0f};
                    appendToBinaryBuffer(&normalToWrite, sizeof(fe::Vec3f));
                }
                std::size_t normalsByteLength = part.vertices.size() * sizeof(fe::Vec3f);
                JsonBox::Object normBvJson;
                normBvJson["buffer"] = 0.0;
                normBvJson["byteOffset"] = static_cast<double>(normalsByteOffset);
                normBvJson["byteLength"] = static_cast<double>(normalsByteLength);
                normBvJson["target"] = static_cast<double>(TARGET_ARRAY_BUFFER);
                gltfBufferViews.emplace_back(normBvJson);
                int normBvIndex = static_cast<int>(gltfBufferViews.size() - 1);
                JsonBox::Object normAccJson;
                normAccJson["bufferView"] = static_cast<double>(normBvIndex);
                normAccJson["byteOffset"] = 0.0;
                normAccJson["componentType"] = static_cast<double>(COMPONENT_TYPE_FLOAT);
                normAccJson["count"] = static_cast<double>(part.vertices.size());
                normAccJson["type"] = "VEC3";
                // Min/max for normals are optional in glTF but can be included
                gltfAccessors.emplace_back(normAccJson);
                attributesJson["NORMAL"] = static_cast<double>(gltfAccessors.size() - 1);
            }
            // TexCoords Accessor (if present)
            if(part.hasTexCoords0){
                std::size_t texCoordsByteOffset = binaryBufferData.size();
                fe::Vec2f minUV = {
                    std::numeric_limits<float>::max(), 
                    std::numeric_limits<float>::max()
                };
                fe::Vec2f maxUV = {
                    std::numeric_limits<float>::lowest(), 
                    std::numeric_limits<float>::lowest()
                };
                for(const fe::gltf::Vertex& v : part.vertices){
                    fe::Vec2f uvToWrite = v.texCoord0 ? *v.texCoord0 : fe::Vec2f{0.0f, 0.0f};
                    appendToBinaryBuffer(&uvToWrite, sizeof(fe::Vec2f));
                    minUV.x = std::min(minUV.x, uvToWrite.x); minUV.y = std::min(minUV.y, uvToWrite.y);
                    maxUV.x = std::max(maxUV.x, uvToWrite.x); maxUV.y = std::max(maxUV.y, uvToWrite.y);
                }
                std::size_t texCoordsByteLength = part.vertices.size() * sizeof(fe::Vec2f);

                JsonBox::Object tcBvJson;
                tcBvJson["buffer"] = 0.0;
                tcBvJson["byteOffset"] = static_cast<double>(texCoordsByteOffset);
                tcBvJson["byteLength"] = static_cast<double>(texCoordsByteLength);
                tcBvJson["target"] = static_cast<double>(TARGET_ARRAY_BUFFER);
                gltfBufferViews.emplace_back(tcBvJson);
                int tcBvIndex = static_cast<int>(gltfBufferViews.size() - 1);

                JsonBox::Object tcAccJson;
                tcAccJson["bufferView"] = static_cast<double>(tcBvIndex);
                tcAccJson["byteOffset"] = 0.0;
                tcAccJson["componentType"] = static_cast<double>(COMPONENT_TYPE_FLOAT);
                tcAccJson["count"] = static_cast<double>(part.vertices.size());
                tcAccJson["type"] = "VEC2";
                JsonBox::Array minUvArr; 
                minUvArr.emplace_back(static_cast<double>(minUV.x)); 
                minUvArr.emplace_back(static_cast<double>(minUV.y));
                JsonBox::Array maxUvArr; 
                maxUvArr.emplace_back(static_cast<double>(maxUV.x)); 
                maxUvArr.emplace_back(static_cast<double>(maxUV.y));
                tcAccJson["min"] = minUvArr;
                tcAccJson["max"] = maxUvArr;
                gltfAccessors.emplace_back(tcAccJson);
                attributesJson["TEXCOORD_0"] = static_cast<double>(gltfAccessors.size() - 1);
            }
            // Indices Accessor
            std::size_t indicesByteOffset = binaryBufferData.size();
            unsigned int maxIndexVal = 0;
            for(unsigned int idx : part.indices){
                maxIndexVal = std::max(maxIndexVal, idx);
            }
            int indicesComponentType;
            std::size_t indexSize;
            // Check if USHORT is enough
            if(maxIndexVal < std::numeric_limits<unsigned short>::max()){
                indicesComponentType = COMPONENT_TYPE_UNSIGNED_SHORT;
                indexSize = sizeof(unsigned short);
                for(unsigned int idx : part.indices){
                    unsigned short shortIdx = static_cast<unsigned short>(idx);
                    appendToBinaryBuffer(&shortIdx, indexSize);
                }
            }else{
                indicesComponentType = COMPONENT_TYPE_UNSIGNED_INT;
                indexSize = sizeof(unsigned int);
                for(unsigned int idx : part.indices){
                    appendToBinaryBuffer(&idx, indexSize);
                }
            }
            std::size_t indicesByteLength = part.indices.size() * indexSize;
            JsonBox::Object idxBvJson;
            idxBvJson["buffer"] = 0.0;
            idxBvJson["byteOffset"] = static_cast<double>(indicesByteOffset);
            idxBvJson["byteLength"] = static_cast<double>(indicesByteLength);
            idxBvJson["target"] = static_cast<double>(TARGET_ELEMENT_ARRAY_BUFFER);
            gltfBufferViews.emplace_back(idxBvJson);
            int idxBvIndex = static_cast<int>(gltfBufferViews.size() - 1);
            JsonBox::Object idxAccJson;
            idxAccJson["bufferView"] = static_cast<double>(idxBvIndex);
            idxAccJson["byteOffset"] = 0.0;
            idxAccJson["componentType"] = static_cast<double>(indicesComponentType);
            idxAccJson["count"] = static_cast<double>(part.indices.size());
            idxAccJson["type"] = "SCALAR";
            JsonBox::Array minIdxArr;
            // Min index is always 0 for this local part
            minIdxArr.emplace_back(0.0);
            JsonBox::Array maxIdxArr;
            maxIdxArr.emplace_back(static_cast<double>(maxIndexVal));
            idxAccJson["min"] = minIdxArr;
            idxAccJson["max"] = maxIdxArr;
            gltfAccessors.emplace_back(idxAccJson);
            int indicesAccessorIndex = static_cast<int>(gltfAccessors.size() - 1);
            // Primitive
            primitiveJson["attributes"] = attributesJson;
            primitiveJson["indices"] = static_cast<double>(indicesAccessorIndex);
            primitiveJson["mode"] = static_cast<double>(PRIMITIVE_MODE_TRIANGLES);
            if(part.materialIndex >= 0 && static_cast<std::size_t>(part.materialIndex) < gltfMaterials.size()){
                primitiveJson["material"] = static_cast<double>(part.materialIndex);
            }
            // Mesh (containing this one primitive)
            JsonBox::Object meshJson;
            meshJson["name"] = part.name;
            JsonBox::Array primitivesArray;
            primitivesArray.emplace_back(primitiveJson);
            meshJson["primitives"] = primitivesArray;
            gltfMeshes.emplace_back(meshJson);
        }
        for(const auto& node:scene.nodes){
            JsonBox::Object nodeJson;
            if(node.name){
                nodeJson["name"] = *node.name;
            }
            if(node.mesh){
                nodeJson["mesh"] = *node.mesh;
            }else if(node.children){
                JsonBox::Array children;
                children.reserve(node.children->size());
                for(const auto& idx : *node.children){
                    children.push_back(idx);
                }
                nodeJson["children"] = children;
            }
            if(node.extra){
                nodeJson["extras"] = JsonBox::Value(*node.extra);
            }
            gltfNodes.emplace_back(nodeJson);
        }
        sceneNodes.emplace_back(static_cast<double>(scene.nodes.size() - 1));
        // Final Assembly of glTF Root
        if(!binaryBufferData.empty()){
            JsonBox::Object bufferJson;
            bufferJson["byteLength"] = static_cast<double>(binaryBufferData.size());
            bufferJson["uri"] = "data:application/octet-stream;base64," + fe::encodeToBase64(binaryBufferData);
            JsonBox::Array buffersArray;
            buffersArray.emplace_back(bufferJson);
            gltfRoot["buffers"] = buffersArray;
        }
        if(!gltfBufferViews.empty()){
            gltfRoot["bufferViews"] = gltfBufferViews;
        }
        if(!gltfAccessors.empty()){
            gltfRoot["accessors"] = gltfAccessors;
        }
        if(!gltfImages.empty()){
            gltfRoot["images"] = gltfImages;
        }
        if(!gltfSamplers.empty()){
            gltfRoot["samplers"] = gltfSamplers;
        }
        if(!gltfTextures.empty()){
            gltfRoot["textures"] = gltfTextures;
        }
        if(!gltfMaterials.empty()){
            gltfRoot["materials"] = gltfMaterials;
        }
        if(!gltfMeshes.empty()){
            gltfRoot["meshes"] = gltfMeshes;
        }
        if(!gltfNodes.empty()){
            gltfRoot["nodes"] = gltfNodes;
        }
        defaultScene["nodes"] = sceneNodes;
        gltfScenes.emplace_back(defaultScene);
        if(!gltfScenes.empty()){
            gltfRoot["scenes"] = gltfScenes;
        }
        bool khrTransmissionUsed = false;
        bool khrVolumeUsed = false;
        bool khrClearcoatUsed = false;
        bool khrMaterialsIORUsed = false;
        bool khrMaterialsEmissiveStrength = false;
        for(const auto& matInfo : scene.materials){
            if(matInfo.khrMaterialsTransmission){
                khrTransmissionUsed = true;
            }
            if(matInfo.khrMaterialsVolume){
                khrVolumeUsed = true;
            }
            if(matInfo.khrMaterialsClearcoat){
                khrClearcoatUsed = true;
            }
            if(matInfo.khrMaterialsIOR){
                khrMaterialsIORUsed = true;
            }
            if(matInfo.khrMaterialsEmissiveStrength){
                khrMaterialsEmissiveStrength = true;
            }
        }
        JsonBox::Array extensionsUsedArray;
        if(khrTransmissionUsed){
            extensionsUsedArray.emplace_back("KHR_materials_transmission");
        }
        if(khrVolumeUsed){
            extensionsUsedArray.emplace_back("KHR_materials_volume");
        }
        if(khrClearcoatUsed){
            extensionsUsedArray.emplace_back("KHR_materials_clearcoat");
        }
        if(khrMaterialsIORUsed){
            extensionsUsedArray.emplace_back("KHR_materials_ior");
        }
        if(khrMaterialsEmissiveStrength){
            extensionsUsedArray.emplace_back("KHR_materials_emissive_strength");
        }
        if(!extensionsUsedArray.empty()){
            gltfRoot["extensionsUsed"] = extensionsUsedArray;
        }
        return JsonBox::Value(gltfRoot);
    }
    std::string toJsonStr(const JsonBox::Value& json){
        std::stringstream ss;
        json.writeToStream(ss,false, false);
        return ss.str();
    }
} // namespace fe::gltf
