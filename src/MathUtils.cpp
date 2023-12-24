#include <MathUtils.hpp>

namespace fe::Math{
	float magnitude(const sf::Vector2f& v) noexcept{
		return std::sqrt((v.x * v.x) + (v.y * v.y));
	}
	sf::Vector2f normalize(const sf::Vector2f& v) noexcept{
		auto m = magnitude(v);
		if(m > 0.0){
			return sf::Vector2f(v.x/m, v.y/m);
		}
		return sf::Vector2f(0.0, 0.0);
	}
	float angle(const sf::Vector2f& v, const sf::Vector2f& origin) noexcept{
		auto angle = std::atan2((v.y - origin.y), (v.x - origin.x)) + degreesToRadians(90);
		if(angle < 0){
			angle += 2.0 * Math::PI;
		}
		if(angle > 2.0 * Math::PI){
			angle -= 2.0 * Math::PI;
		}
		return angle;
	}
	float directedAngle(const sf::Vector2f& v1, const sf::Vector2f& v2, const sf::Vector2f& origin) noexcept{
		auto angle = std::atan2(v1.y - origin.y, v1.x - origin.x) - std::atan2(v2.y - origin.y, v2.x - origin.x);
		if(angle < 0){
			angle += 2.0 * Math::PI;
		}
		if(angle > 2.0 * Math::PI){
			angle -= 2.0 * Math::PI;
		}
		return angle;
	}
	constexpr float degreesToRadians(float d) noexcept{
		return (d * PI/180.f);
	}
	constexpr float radiansToDegrees(float r) noexcept{
		return (r * 180.f/PI);
	}
}
