#include <Image.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <limits>
#include <string>
#include <sstream>

namespace fe{
    Image::Image() noexcept
    : imageData()
    , mWidth(0)
    , mHeight(0){}
    void Image::create(std::size_t width, std::size_t height, const sf::Color& color) noexcept{
        if(!imageData.empty()){
            imageData.clear();
        }
        mWidth = width;
        mHeight = height;
        imageData.resize(mWidth * mHeight * 4);
        for(auto x=0u;x<mWidth;++x){
            for(auto y=0u;y<mHeight;++y){
                setPixel(x, y, color);
            }
        }
    }
    void Image::setPixel(const sf::Vector2f& pos, const sf::Color& color) noexcept{
        setPixel(pos.x, pos.y, color);
    }
    sf::Vector2f Image::getSize() const noexcept{
        return sf::Vector2f(mWidth, mHeight);
    }
    void Image::setPixel(std::size_t x, std::size_t y, const sf::Color& color) noexcept{
        if(!imageData.empty()){
            std::size_t index = (y * mWidth + x) * 4;
            imageData[index + 0] = color.r;
            imageData[index + 1] = color.g;
            imageData[index + 2] = color.b;
            imageData[index + 3] = color.a;
        }
    }
    bool isBase64(unsigned char c){
        return (std::isalnum(c) || (c == '+') || (c == '/'));
    }
    std::string encodeToBase64(const std::vector<std::uint8_t>& data){
        if(data.empty()){
            return "";
        }
        static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                "abcdefghijklmnopqrstuvwxyz"
                                                "0123456789+/";
        std::stringstream ss;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        auto in_len = data.size();
        auto bytes_to_encode = data.data();
        while(in_len--){
            char_array_3[i++] = *(bytes_to_encode++);
            if(i == 3){
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
    
                for(i = 0; i < 4; i++){
                    ss << base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
        if(i > 0){
            for(j = i; j < 3; j++){
                char_array_3[j] = '\0';
            }
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(j = 0; j < i + 1; j++){
                ss << base64_chars[char_array_4[j]];
            }
            while((i++ < 3)){
                ss << '=';
            }
        }
        return ss.str();
    }    
    std::vector<std::uint8_t> encodeImageToPngInMemory(const Image& image){
        int pngDataLength = 0;
        int width = image.getSize().x;
        int height = image.getSize().y;
        int stride_in_bytes = width * 4;
        unsigned char *pngDataPtr = stbi_write_png_to_mem(
            image.imageData.data(),
            stride_in_bytes,
            width,
            height,
            4,
            &pngDataLength
        );
        if(!pngDataPtr || pngDataLength == 0){
            if(pngDataPtr){
                STBIW_FREE(pngDataPtr);
            }
            return {};
        }
        std::vector<std::uint8_t> pngEncodedData(pngDataPtr, pngDataPtr + pngDataLength);
        STBIW_FREE(pngDataPtr);    
        return pngEncodedData;
    }
}