#include <3D/GLTF/TextureInfo.hpp>

#include <Image.hpp>

namespace fe::gltf{
    TextureInfo::TextureInfo(const std::string& tex_name, const std::string& base64_data_uri)
    : name(tex_name)
    , uri(std::move(base64_data_uri)){}
    TextureInfo TextureInfo::createFromImage(const std::string& tex_name, const fe::Image& raw_image){
        auto textureUri = "data:image/png;base64," + fe::encodeToBase64(fe::encodeImageToPngInMemory(raw_image));
        return TextureInfo(tex_name, textureUri);
    }
}