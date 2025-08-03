#ifndef FLOWER_EVOLVER_IMAGE_HPP
#define FLOWER_EVOLVER_IMAGE_HPP

#include <vector>
#include <memory>
#include <cstdint>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace fe{
	/**
	 * @brief simple Image to replace the one from sfml for emscripten.
	 */
	struct Image final{
		/**
		 * @brief default constructor
		 */
		Image() noexcept;
		/**
		 * @brief allocates the memory needed for the image
		 * @param width std::size_t width of the image
		 * @param height std::size_t height of the image
		 * @param color default color to set
		 */
		void create(std::size_t width, std::size_t height, const sf::Color& color) noexcept;
		/**
		 * @brief sets the pixel color
		 * @param pos const sf::Vector2f& position
		 * @param color const sf::Color& color
		 */
		void setPixel(const sf::Vector2f& pos, const sf::Color& color) noexcept;
		/**
		 * @brief gets image size
		 * @return sf::Vector2f
		 */
		sf::Vector2f getSize() const noexcept;
		/**
		 * @brief sets the pixel color
		 * @param x std::size_t x position
                 * @param y std::size_t y position
		 * @param color const sf::Color& color
		 */
		void setPixel(std::size_t x, std::size_t y, const sf::Color& color) noexcept;
		/**
		 * @brief gets the pixel color
		 * @param x std::size_t x position
                 * @param y std::size_t y position
		 * @return sf::Color
		 */
		inline sf::Color getPixel(std::size_t x, std::size_t y) const noexcept{
			std::size_t index = (y * mWidth + x) * 4;
			if(!imageData.empty() &&  (index + 3) < imageData.size() && (index >= 0)){
				return sf::Color(
					imageData[index + 0], 
					imageData[index + 1], 
					imageData[index + 2], 
					imageData[index + 3]
				);
			}
			return {};
		}
		// data
		std::vector<std::uint8_t> imageData;
		std::size_t mWidth;
		std::size_t mHeight;
	};
    /**
     * @brief checks if is a base64 char
     * @param c unsigned char
     * @return bool
     */
    inline bool isBase64(unsigned char c);
    /**
     * @brief encodes an image to base64
     * @param data std::vector<std::uint8_t>&
     * @return std::string base64 image 
     */
    std::string encodeToBase64(const std::vector<std::uint8_t>& data);
    /**
     * @brief Encodes raw image data into a PNG byte stream in memory.
     *
     * @param image The Image object containing raw pixel data. expected RBGA
     * @return A std::vector<std::uint8_t> containing the PNG-encoded data.
     *         Returns an empty vector if encoding fails or the input image is invalid.
     */
    std::vector<std::uint8_t> encodeImageToPngInMemory(const Image& image);
} // namespace fe

#endif // FLOWER_EVOLVER_IMAGE_HPP
