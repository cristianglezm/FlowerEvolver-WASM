#ifndef FLOWER_EVOLVER_3D_VEC3_HPP
#define FLOWER_EVOLVER_3D_VEC3_HPP

#include <SFML/System/Vector2.hpp>

#include <limits>
#include <cmath>

namespace fe{
    using Vec2f = sf::Vector2f;
    using Vec2i = sf::Vector2i;
    /**
     * @brief A templated 3D vector class for mathematical and geometric computations.
     * 
     * Provides basic operations for 3D vector manipulation, such as normalization 
     * and calculating vector length and length squared. It is templated to support 
     * various numerical types, such as `float`, `double`, or `int`.
     * 
     * @tparam T The type of the vector components (e.g., float, double, int).
     */
    template<typename T>
    struct Vec3 final{
        static_assert(std::is_arithmetic_v<T>, "Vec3 T needs to be a number.");
        /**
         * @brief Default constructor initializes the vector to (0, 0, 0).
         */
        Vec3();
        /**
         * @brief Parameterized constructor initializes the vector with specific values.
         * 
         * @param x_ The x-component of the vector.
         * @param y_ The y-component of the vector.
         * @param z_ The z-component of the vector.
         */
        Vec3(T x_, T y_, T z_);
        /**
         * @brief Normalizes the vector to have a magnitude of 1.
         * 
         * If the vector is zero or its magnitude is very small, all components 
         * will be set to 0 to avoid division by zero.
         */
        void normalize();
        /**
         * @brief Computes the squared length of the vector.
         * @return The squared length of the vector.
         */
        T lengthSquared() const;
        /**
         * @brief Computes the actual length (magnitude) of the vector.
         * 
         * The length is calculated as the square root of the sum of the 
         * squares of the components: `sqrt(x*x + y*y + z*z)`.
         * 
         * @return The length of the vector.
         */
        T length() const;
        /**
         * @brief Computes the dot product of this vector with another vector.
         * @param other const Vec3<T>& The other vector.
         * @return The dot product (a scalar value).
         */
        T dot(const Vec3<T>& other) const;
        /**
         * @brief Computes the cross product of this vector with another vector.
         * @param other const Vec3<T>& The other vector.
         * @return A new Vec3<T> representing the cross product (this x other).
         */
        Vec3<T> cross(const Vec3<T>& other) const;
        Vec3<T>& operator+=(const Vec3<T>& rhs);
        Vec3<T>& operator-=(const Vec3<T>& rhs);
        Vec3<T>& operator*=(T scalar);
        Vec3<T>& operator/=(T scalar);
        Vec3<T> operator-() const;
        // data 
        T x;
        T y;
        T z;
    };
    template<typename T>
    inline Vec3<T> operator+(Vec3<T> lhs, const Vec3<T>& rhs) noexcept;
    template<typename T>
    inline Vec3<T> operator-(Vec3<T> lhs, const Vec3<T>& rhs) noexcept;
    template<typename T>
    inline Vec3<T> operator*(Vec3<T> lhs, T scalar) noexcept;
    template<typename T>
    inline Vec3<T> operator*(T scalar, Vec3<T> rhs) noexcept;
    template<typename T>
    inline Vec3<T> operator/(Vec3<T> lhs, T scalar) noexcept;
    using Vec3f = Vec3<float>;
    /**
     * @brief simple vec4 for material colors
     */
    struct Vec4f{
        Vec4f(float _r, float _g, float _b, float _a)
        : r{_r}
        , g{_g}
        , b{_b}
        , a{_a}{}
        // data
        float r{0.0};
        float g{0.0};
        float b{0.0};
        float a{0.0};
    };
} // namespace fe

#include "Vec.inl"

#endif // FLOWER_EVOLVER_3D_VEC3_HPP