#include <fe/MathUtils.hpp>

namespace fe::Math{
	float magnitude(const fe::Vector2f& v) noexcept{
		return std::sqrt((v.x * v.x) + (v.y * v.y));
	}
	fe::Vector2f normalize(const fe::Vector2f& v) noexcept{
		auto m = magnitude(v);
		if(m > 0.0){
			return fe::Vector2f(v.x/m, v.y/m);
		}
		return fe::Vector2f(0.0, 0.0);
	}
	float angle(const fe::Vector2f& v, const fe::Vector2f& origin) noexcept{
		auto angle = std::atan2((v.y - origin.y), (v.x - origin.x)) + degreesToRadians(90);
		if(angle < 0){
			angle += 2.0 * Math::PI;
		}
		if(angle > 2.0 * Math::PI){
			angle -= 2.0 * Math::PI;
		}
		return angle;
	}
	float directedAngle(const fe::Vector2f& v1, const fe::Vector2f& v2, const fe::Vector2f& origin) noexcept{
		auto angle = std::atan2(v1.y - origin.y, v1.x - origin.x) - std::atan2(v2.y - origin.y, v2.x - origin.x);
		if(angle < 0){
			angle += 2.0 * Math::PI;
		}
		if(angle > 2.0 * Math::PI){
			angle -= 2.0 * Math::PI;
		}
		return angle;
	}
}
