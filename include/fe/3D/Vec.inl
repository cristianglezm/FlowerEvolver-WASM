namespace fe{
    template<typename T>
    inline Vec3<T> operator+(Vec3<T> lhs, const Vec3<T>& rhs) noexcept{
        lhs += rhs;
        return lhs;
    }
    template<typename T>
    inline Vec3<T> operator-(Vec3<T> lhs, const Vec3<T>& rhs) noexcept{
        lhs -= rhs;
        return lhs;
    }
    template<typename T>
    inline Vec3<T> operator*(Vec3<T> lhs, T scalar) noexcept{
        lhs *= scalar;
        return lhs;
    }
    template<typename T>
    inline Vec3<T> operator*(T scalar, Vec3<T> rhs) noexcept{
        rhs *= scalar;
        return rhs;
    }
    template<typename T>
    inline Vec3<T> operator/(Vec3<T> lhs, T scalar) noexcept{
        lhs /= scalar;
        return lhs;
    }
    template<typename T>
    Vec3<T>::Vec3()
    : x{}
    , y{}
    , z{}{}
    template<typename T>
    Vec3<T>::Vec3(T x_, T y_, T z_)
    : x(x_)
    , y(y_)
    , z(z_){}
    template<typename T>
    void Vec3<T>::normalize(){
        T magSq = x * x + y * y + z * z;
        if(magSq > std::numeric_limits<T>::epsilon()){
            T mag = std::sqrt(magSq);
            x /= mag;
            y /= mag;
            z /= mag;
        }else{
            x = 0;
            y = 0;
            z = 0;
        }
    }
    template<typename T>
    T Vec3<T>::lengthSquared() const{
        return x * x + y * y + z * z;
    }
    template<typename T>
    T Vec3<T>::length() const{
        return std::sqrt(lengthSquared());
    }
    template<typename T>
    T Vec3<T>::dot(const Vec3<T>& other) const{
        return x * other.x + y * other.y + z * other.z;
    }
    template<typename T>
    Vec3<T> Vec3<T>::cross(const Vec3<T>& other) const{
        return Vec3<T>(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    template<typename T>
    Vec3<T>& Vec3<T>::operator+=(const Vec3<T>& rhs){
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    template<typename T>
    Vec3<T>& Vec3<T>::operator-=(const Vec3<T>& rhs){
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    template<typename T>
    Vec3<T>& Vec3<T>::operator*=(T scalar){
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    template<typename T>
    Vec3<T>& Vec3<T>::operator/=(T scalar){
        if(std::abs(scalar) < std::numeric_limits<T>::epsilon()){
            x = static_cast<T>(0);
            y = static_cast<T>(0);
            z = static_cast<T>(0);
        }else{
            x /= scalar;
            y /= scalar;
            z /= scalar;
        }
        return *this;
    }
    template<typename T>
    Vec3<T> Vec3<T>::operator-() const{
        return Vec3<T>(-x, -y, -z);
    }
}