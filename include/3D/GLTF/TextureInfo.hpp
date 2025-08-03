#ifndef FLOWER_EVOLVER_3D_GLTF_TEXTURE_INFO_HPP
#define FLOWER_EVOLVER_3D_GLTF_TEXTURE_INFO_HPP

#include <string>
#include <Image.hpp>

namespace fe::gltf{
    /**
     * @brief Stores information about a texture image to be used in glTF.
     * Can hold raw image data for processing or the final Base64 URI.
     */
    struct TextureInfo{
        /**
         * @brief Constructor for an embedded Base64 texture.
         * @param tex_name The name of the texture.
         * @param base64_data_uri The full "data:image/png;base64,..." string.
         */
        TextureInfo(const std::string& tex_name, const std::string& base64_data_uri);
        /**
         * @brief Static helper to create fe::gltf::TextureInfo by processing an fe::Image.
         * @param tex_name The name of the texture.
         * @param raw_image The fe::Image object containing raw pixel data.
         * @return TextureInfo with the URI field populated.
         */
        static TextureInfo createFromImage(const std::string& tex_name, const fe::Image& raw_image);
        // data
        std::string name;
        std::string mimeType = "image/png";
        std::string uri;
    };
} // namespace fe::gltf

#endif // FLOWER_EVOLVER_3D_GLTF_TEXTURE_INFO_HPP