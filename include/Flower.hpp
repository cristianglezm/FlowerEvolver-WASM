#ifndef FLOWER_HPP
#define FLOWER_HPP

#include <functional>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include <JsonBox.h>

#include <DNA.hpp>
#include <Petals.hpp>

namespace fe{
	/**
	 *  @brief Flower struct
	 */
	struct Flower{
		/**
		 *  @brief default constructor
		 */
		Flower();
		/**
		 *  @brief constructor to use when creating a random flower.
		 *  
		 *  @param [in] pos       position
		 *  @param [in] radius    radius
		 *  @param [in] numLayers number of layers
		 *  @param [in] P         P
		 *  @param [in] bias      bias
		 */
		Flower(const sf::Vector2f& pos, int radius, int numLayers, float P, float bias);
		/**
		 *  @brief constructor to use when creating a child flower.
		 *  
		 *  @param [in] pos       position
		 *  @param [in] radius    radius
		 *  @param [in] numLayers number of layers
		 *  @param [in] P         P
		 *  @param [in] bias      bias
		 *  @param [in] dna       fe::DNA to use to make this flower
		 */
		Flower(const sf::Vector2f& pos, int radius, int numLayers, float P, float bias, DNA&& dna);
		/**
		 *  @brief copy constructor
		 */
		Flower(const Flower& rhs) noexcept;
		/**
		 *  @brief move constructor
		 */
		Flower(Flower&& rhs) noexcept;
		/**
		 *  @brief load a JsonBox::Object with ["Flower"]
		 *  
		 *  @param [in] o JsonBox::Object
		 */
		Flower(JsonBox::Object o);
		/**
		 *  @brief converts Flower to JsonBox::Value
		 *  
		 *  @return JsonBox::Value
		 */
		JsonBox::Value toJson() const noexcept;
		void operator=(Flower&& rhs) noexcept;
		void operator=(const Flower& rhs) noexcept;
	//data
		DNA dna;
		Petals petals;
	};
}
#endif // FLOWER_HPP