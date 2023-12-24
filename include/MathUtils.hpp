#ifndef FLOWER_EVOLVER_MATH_UTILS_HPP
#define FLOWER_EVOLVER_MATH_UTILS_HPP

#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace fe::Math{
	/**
	 *  @brief PI
	 */
	constexpr float PI = 3.14159265359;
	/**
	 * @brief
	 * @param 
	 */
	float magnitude(const sf::Vector2f& v) noexcept;
	/**
	 *  @brief returns a normalized v
	 *  
	 *  @param [in] v vector
	 *  @return sf::Vector2f normalized sf::Vector2f v
	 */
	sf::Vector2f normalize(const sf::Vector2f& v) noexcept;
	/**
	 *  @brief get the difference between v and origin in radians
	 *  clockwise direction and -y is 0º degrees
	 *  @code
	 *      // angle is 281.31º from 0,0 to 5,1 in clockwise direction
	 *      auto angle = Math::radiansToDegrees(Math::angle({0,0},{5,1}));
	 *      // angle is 101.31º from 5,1 to 0,0 in clockwise direction
	 *      angle = Math::radiansToDegrees(Math::angle({5,1},{0,0}));
	 *      //
	 *  @endcode
	 *  @param [in] v sf::Vector2f
	 *  @param [in] origin sf::Vector2f Origin which of sf::Vector2f v
	 *  @return float angle in radians
	 */
	float angle(const sf::Vector2f& v, const sf::Vector2f& origin = sf::Vector2f(0.f,0.f)) noexcept;
	/**
	 *  @brief get the difference from v1 and v2 with origin in radians
	 *  clockwise direction and -y is 0º degrees
	 *  @code
	 *      // angle is 326.31º degrees from [5,1] to [3,3] with origin [0,0]
	 *      auto angle = Math::radiansToDegrees(Math::directedAngle({5,1},{3,3},{0,0}));
	 *      // angle is 33.6901º degrees from [3,3] to [5,1] with origin [0,0]
	 *      angle = Math::radiansToDegrees(Math::directedAngle({3,3},{5,1},{0,0}));
	 *      // vector [5,1] with origin [3,3] is at 315º degrees from [3,3]
	 *      angle = Math::radiansToDegrees(Math::directedAngle({5,1},{3,3},{3,3}));
	 *  @endcode
	 *  @param [in] v1 const sf::Vector2f&
	 *  @param [in] v2 const sf::Vector2f&
	 *  @param [in] o  const sf::Vector2f&
	 *  @return float angles in radians
	 */
	float directedAngle(const sf::Vector2f& v1, const sf::Vector2f& v2, const sf::Vector2f& origin = sf::Vector2f(0,0)) noexcept;
	/**
	 *  @brief converts to radians
	 *  
	 *  @param [in] d float degrees
	 *  @return float
	 */
	constexpr float degreesToRadians(float d) noexcept;
	/**
	 *  @brief converts to degrees
	 *  
	 *  @param [in] r float radians
	 *  @return float
	 */
	constexpr float radiansToDegrees(float r) noexcept;
}

#endif // FLOWER_EVOLVER_MATH_UTILS_HPP