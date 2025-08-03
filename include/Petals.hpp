#ifndef FLOWER_EVOLVER_PETALS_HPP
#define FLOWER_EVOLVER_PETALS_HPP

#include <memory>
#include <vector>

#include <Image.hpp>
#include <MathUtils.hpp>

#include <EvoAI.hpp>
#include <JsonBox.h>

namespace fe{
	/**
	 *  @brief Petals component
	 *  @code
	 *      Petals p(64,3,6.0,1.0);
	 *      // genome is a CPPN and has 4 inputs and 4 outputs
	 *      draw(Petals::Type::TrunkAndPetals, p, genomes[index]);
	 *  @endcode
	 */
	struct Petals final{
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
	 * @brief Determines the number of times a number can be divided by a given divisor until it is <= 1.
	 *
	 * @param [in] val The number to be divided.
	 * @param [in] divisor The divisor applied on each iteration.
	 * @return The count of divisions performed until the value is ≤ 1.
	 */
	int getTimesDivisibleBy(int val, int divisor) noexcept;
	/**
	 *  @brief will draw the layer of a petal, 
	 *  
	 *  @param [in] petals Petals
	 *  @param [in] g      a cppn EvoAI::Genome with 4 inputs 4 outputs
	 *  @param [in] layer  the layer to draw
	 *  @param [in] applyLayeredRadiusScaling it will divide the radius / 2.0 from petals.numLayers to layer
	 */
	void drawLayer(Petals& petals, EvoAI::Genome& g, int layer, bool applyLayeredRadiusScaling = true) noexcept;
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
