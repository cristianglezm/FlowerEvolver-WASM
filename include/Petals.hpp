#ifndef FLOWER_EVOLVER_PETALS_HPP
#define FLOWER_EVOLVER_PETALS_HPP

#include <memory>
#include <vector>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <MathUtils.hpp>

#include <EvoAI.hpp>

#include <JsonBox.h>


namespace fe{
	/**
	 * @brief simple Image to replace the one from sfml for emscripten.
	 */
	struct Image final{
		Image()
		: imageData()
		, mWidth(0)
		, mHeight(0){}
		void create(std::size_t width, std::size_t height, const sf::Color& color) noexcept{
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
		void setPixel(const sf::Vector2f& pos, const sf::Color& color) noexcept{
			setPixel(pos.x, pos.y, color);
		}
		sf::Vector2f getSize() noexcept{
			return sf::Vector2f(mWidth, mHeight);
		}
		void setPixel(std::size_t x, std::size_t y, const sf::Color& color) noexcept{
			if(!imageData.empty()){
				std::size_t index = (y * mWidth + x) * 4;
				imageData[index + 0] = color.r;
				imageData[index + 1] = color.g;
				imageData[index + 2] = color.b;
				imageData[index + 3] = color.a;
			}
		}
		// data
		std::vector<std::uint8_t> imageData;
		std::size_t mWidth;
		std::size_t mHeight;
	};
	/**
	 *  @brief Petals component
	 *  @code
	 *      Petals p(64,3,6.0,1.0);
	 *      //use draw before calling loadImageAndSetTexture
	 *      // genome is CPPN and has 4 inputs and 4 outputs
	 *      draw(Petals::Type::TrunkAndPetals, p, genomes[index]);
	 *      p.loadImageAndSetTexture();
	 *  @endcode
	 */
	struct Petals{
		/**
		 *  @brief Type enum for draw function
		 */
		enum class Type : int{
			Trunk,
			Petals,
			TrunkAndPetals
		};
		/**
		 *  @brief Default constructor
		 */
		Petals() noexcept;
		/**
		 *  @brief constructor
		 *  @param r int radius in pixels
		 *  @param nLayers int number of Layers
		 *  @param P float parameter for NN
		 *  @param bias flat bias for NN
		 */
		Petals(int r, int nLayers, float P, float bias);
		/**
		 *  @brief load JsonBox::Object
		 *  @param o JsonBox::Object
		 */
		Petals(JsonBox::Object o);
		/**
		 *  @brief copy constructor
		 */
		Petals(const Petals& rhs) noexcept;
		/**
		 *  @brief move constructor
		 */
		Petals(Petals&& rhs) noexcept;
		/**
		 *  @brief converts the object to JsonBox::Value
		 *  @return JsonBox::Value
		 */
		JsonBox::Value toJson() const noexcept;
		//operators
		void operator=(const Petals& rhs) noexcept;
		void operator=(Petals&& rhs) noexcept;
		//data
		fe::Image image;
		float bias;
		float P;
		int radius;
		int numLayers;
		bool hasBloom;
	};
	namespace priv{
		/**
		*  @brief queries the neural network
		 *  
		 *  @param [in] nn            EvoAI::NeuralNetwork with 4 inputs 4 outputs
		 *  @param [in] petals        Petals that is being processed
		 *  @param [in] pos           position
		 *  @param [in] currentRadius current radius
		 *  @param [in] currentLayer  current layer
		 *  @return std::vector<double> results
		 */
		std::vector<double> queryNN(EvoAI::NeuralNetwork& nn, Petals& petals, const sf::Vector2f& pos, int currentRadius, int currentLayer) noexcept;
		/**
		 *  @brief sets color for every pixel in a line from petals.origin to pos as maximum if no cut is made.
		 *  
		 *  @param [in] pos           sf::Vector2f& position
		 *  @param [in] nn            EvoAI::NeuralNetwork with 4 inputs and 4 outputs
		 *  @param [in] petals        Petals that is being processed
		 *  @param [in] currentRadius current radius
		 *  @param [in] currentLayer  current layer
		 */
		void setColorAndCut(const sf::Vector2f& pos, EvoAI::NeuralNetwork& nn, Petals& petals, int currentRadius, int currentLayer) noexcept;
		/**
		 *  @brief It will use setColorAndCut to draw a pattern into petals.
		 *  
		 *  @param [in] origin        center of circle (Flower)
		 *  @param [in] r             max radius
		 *  @param [in] petals        Petals
		 *  @param [in] nn            EvoAI::NeuralNetwork with 4 inputs 4 outputs
		 *  @param [in] currentRadius current Radius
		 *  @param [in] currentLayer  current Layer
		 */
		void EightWaySymmetricSetColor(const sf::Vector2f& origin, const sf::Vector2f& r, Petals& petals, EvoAI::NeuralNetwork& nn, int currentRadius, int currentLayer) noexcept;
		/**
		 *  @brief will draw the flower
		 *  
		 *  @param [in] petals        Petals
		 *  @param [in] nn            EvoAI::NeuralNetwork with 4 inputs 4 outputs
		 *  @param [in] currentRadius current Radius
		 *  @param [in] currentLayer  current Layer
		 */
		void drawPetals(Petals& petals, EvoAI::NeuralNetwork& nn, int currentRadius, int currentLayer) noexcept;
		/**
		 *  @brief will draw a trunk of the flower
		 *  
		 *  @param [in] petals Petals
		 */
		void drawTrunk(Petals& petals) noexcept;
	} // namespace priv/
	/**
	 *  @brief will draw what Petals::Type is
	 *  
	 *  @param [in] t      Petals::Type
	 *  @param [in] petals Petals
	 *  @param [in] g      a cppn EvoAI::Genome with 4 inputs 4 outputs
	 */
	void draw(Petals::Type t, Petals& petals, EvoAI::Genome& g) noexcept;
}
#endif // FLOWER_EVOLVER_PETALS_HPP