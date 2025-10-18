#pragma once
#include <iostream>
#include <cmath>
#include <format>
#include <cstdio>

namespace lxrr {

template<class T>
class Vector3 {
public:
    Vector3(T tx, T ty, T tz):e{tx, ty, tz} {}
    Vector3():e{0,0,0} {}

    T operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i) { return e[i]; }
    T x() const { return e[0]; }
    T y() const { return e[1]; }
    T z() const { return e[2]; }

    Vector3<T> operator+(const Vector3<T> &right) const;
    Vector3<T> operator-(const Vector3<T> &right) const;
    Vector3<T> operator*(const Vector3<T> &right) const;
    Vector3<T> operator/(const Vector3<T> &right) const;
    Vector3<T> operator/(float t) const;
    Vector3<T>& operator+=(const Vector3<T> &right) const;
    Vector3<T>& operator-=(const Vector3<T> &right) const;
    Vector3<T>& operator*=(const Vector3<T> &right) const;
    Vector3<T>& operator/=(const Vector3<T> &right) const;
    Vector3<T> operator*(float t) const;
    template<class U>
    friend Vector3<U> operator*(float t, const Vector3<U> &right);
    Vector3<T>& operator*=(float t) const;

    float length2() const;
    float length() const;
    Vector3<T>& norm();
    Vector3<T> normalized() const;

    float dot(const Vector3<T> &right) const;
    static float dot(const Vector3<T> &left, const Vector3<T> &right);
    static Vector3<T> cross(const Vector3<T> &left, const Vector3<T> &right);
    static Vector3<T> lerp(const Vector3<T> &left, const Vector3<T> &right, float t);

    template<class U>
    friend std::ostream& operator<<(std::ostream& out, const Vector3<U> &v);
    template<class U>
    friend std::istream& operator>>(std::istream& in, Vector3<U> &v);


private:
    T e[3];
};

using vec3 = Vector3<float>;
using vec3f = Vector3<float>;
using vec3d = Vector3<double>;
using vec3i = Vector3<int>;
using point3 = Vector3<float>;
using color3 = Vector3<float>;




template<class T>
class Vector4 {
public:
    Vector4(T tx, T ty, T tz):e{tx, ty, tz}, w(1) {}
    Vector4(T tx, T ty, T tz, T tw):e{tx, ty, tz}, w(tw) {}
    Vector4():e{0,0,0}, w(1) {}
    Vector4(const Vector3<T> &v):e{v}, w(1) {}

    T operator[](size_t i) const { return i == 3 ? w : e[i]; }
    T& operator[](size_t i) { return i == 3 ? w : e[i]; }
    T x() const { return e[0]; }
    T y() const { return e[1]; }
    T z() const { return e[2]; }
    Vector3<T> get_3d() const { return e; }
    T get_w() const { return w; }

    Vector4<T> operator+(const Vector4<T> &right) const;
    Vector4<T> operator-(const Vector4<T> &right) const;
    Vector4<T> operator*(const Vector4<T> &right) const;
    Vector4<T> operator/(const Vector4<T> &right) const;
    Vector4<T> operator/(float t) const;
    // Vector4<T>& operator+=(const Vector4<T> &right) const;
    // Vector4<T>& operator-=(const Vector4<T> &right) const;
    // Vector4<T>& operator*=(const Vector4<T> &right) const;
    // Vector4<T>& operator/=(const Vector4<T> &right) const;
    Vector4<T> operator*(float t) const;
    template<class U>
    friend Vector4<U> operator*(float t, const Vector4<U> &right);
    Vector4<T>& operator*=(float t) const;

    float length2() const;
    float length() const;
    Vector4<T>& norm();
    Vector4<T> normalized() const;
    void standardization()
	{		
		e[0] /= w; e[1] /= w; e[2] /= w;
		w = 1;
	}

    float dot(const Vector4<T> &right) const;
    static float dot(const Vector4<T> &left, const Vector4<T> &right);
    static Vector4<T> cross(const Vector4<T> &left, const Vector4<T> &right);
    static Vector4<T> lerp(const Vector4<T> &left, const Vector4<T> &right, float t);

    template<class U>
    friend std::ostream& operator<<(std::ostream& out, const Vector4<U> &v);
    template<class U>
    friend std::istream& operator>>(std::istream& in, Vector4<U> &v);

private:
    Vector3<T> e;
    T w;
};

using vec4 = Vector4<float>;
using vec4f = Vector4<float>;
using vec4i = Vector4<int>;




class Vector2
{
public:
	Vector2():e{0,0} { }
	Vector2(float tx, float ty):e{tx,ty} { }

    float operator[](size_t i) const { return e[i]; }
    float& operator[](size_t i) { return e[i]; }

    float x() const { return e[0]; }
    float y() const { return e[1]; }

	Vector2 operator + (const Vector2& right) const {
		return Vector2(e[0] + right[0], e[1] + right[1]);
	}
	Vector2 operator - (const Vector2& right) const {
		return Vector2(e[0] - right[0], e[1] - right[1]);
	}
	Vector2 operator * (float value) const {
		return Vector2(e[0] * value, e[1] * value);
	}
	Vector2 operator / (float value) const {
		return Vector2(e[0] / value, e[1] / value);
	}

    static Vector2 lerp(const Vector2& left, const Vector2& right, float t) {
        return Vector2(left[0] + (right[0] - left[0]) * t,
                       left[1] + (right[1] - left[1]) * t);
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector2 &v) {
        return out << std::format("Vec({:.3f},{:.3f})", v[0], v[1]);
    }

private:
	float e[2];
};

using vec2 = Vector2;











////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Implement
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




//////////////////////////////////////////////
// class Vector3
/////////////////////////////////////////////
template<class T>
inline Vector3<T> Vector3<T>::operator+(const Vector3<T> &right) const {
    return Vector3<T>(e[0] + right[0], e[1] + right[1], e[2] + right[2]);
}

template<class T>
inline Vector3<T> Vector3<T>::operator-(const Vector3<T> &right) const {
    return Vector3<T>(e[0] - right[0], e[1] - right[1], e[2] - right[2]);
}

template<class T>
inline Vector3<T> Vector3<T>::operator*(const Vector3<T> &right) const {
    return Vector3<T>(e[0] * right[0], e[1] * right[1], e[2] * right[2]);
}

template<class T>
inline Vector3<T> Vector3<T>::operator/(const Vector3<T> &right) const {
    return Vector3<T>(e[0] / right[0], e[1] / right[1], e[2] / right[2]);
}

template<class T>
inline Vector3<T> Vector3<T>::operator/(float t) const {
    return Vector3<T>(e[0] / t, e[1] / t, e[2] / t);
}

template<class T>
inline Vector3<T>& Vector3<T>::operator+=(const Vector3<T> &right) const {
    e[0] += right[0]; e[1] += right[1]; e[2] += right[2];
    return *this;
}

template<class T>
inline Vector3<T>& Vector3<T>::operator-=(const Vector3<T> &right) const {
    e[0] -= right[0]; e[1] -= right[1]; e[2] -= right[2];
    return *this;
}

template<class T>
inline Vector3<T>& Vector3<T>::operator*=(const Vector3<T> &right) const {
    e[0] *= right[0]; e[1] *= right[1]; e[2] *= right[2];
    return *this;
}

template<class T>
inline Vector3<T>& Vector3<T>::operator/=(const Vector3<T> &right) const {
    e[0] /= right[0]; e[1] /= right[1]; e[2] /= right[2];
    return *this;
}

template<class T>
inline Vector3<T> Vector3<T>::operator*(float t) const {
    return Vector3<T>(e[0] * t, e[1] * t, e[2] * t);
}

template<class T>
inline Vector3<T> operator*(float t, const Vector3<T> &right) {
    return right * t;
}

template<class T>
inline Vector3<T>& Vector3<T>::operator*=(float t) const {
    e[0] *= t; e[1] *= t; e[2] *= t;
    return *this;
}

template<class T>
inline float Vector3<T>::length2() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
}

template<class T>
inline float Vector3<T>::length() const {
    return std::sqrt(length2());
}

template<class T>
inline Vector3<T>& Vector3<T>::norm() {
    *this = *this / length();
    return *this;
}

template<class T>
inline Vector3<T> Vector3<T>::normalized() const {
    return Vector3<T>(e[0] / length(), e[1] / length(), e[2] / length());
}

template<class T>
inline float Vector3<T>::dot(const Vector3<T> &right) const {
    return e[0] * right[0] + e[1] * right[1] + e[2] * right[2];
}

template<class T>
inline float Vector3<T>::dot(const Vector3<T> &left, const Vector3<T> &right) {
    return left.dot(right);
}

template<class T>
inline Vector3<T> Vector3<T>::cross(const Vector3<T> &left, const Vector3<T> &right) {
    return Vector3<T>(left[1] * right[2] - left[2] * right[1],
                      left[2] * right[0] - left[0] * right[2],
                      left[0] * right[1] - left[1] * right[0]);
}

template<class T>
inline Vector3<T> Vector3<T>::lerp(const Vector3<T> &left, const Vector3<T> &right, float t) {
    return left + (right - left) * t;
}

template<class T>
inline std::ostream& operator<<(std::ostream& out, const Vector3<T> &v) {
    out << std::format("Vec({:.3f},{:.3f},{:.3f})", v[0], v[1], v[2]);
    return out;
}

template<class T>
inline std::istream& operator>>(std::istream& in, Vector3<T> &v) {
    in >> v[0] >> v[1] >> v[2];
    return in;
}

//////////////////////////////////////////////
// class Vector3
/////////////////////////////////////////////










//////////////////////////////////////////////
// class Vector4
/////////////////////////////////////////////
template<class T>
inline Vector4<T> Vector4<T>::operator+(const Vector4<T> &right) const {
    return Vector4<T>(e[0] + right[0], e[1] + right[1], e[2] + right[2]);
}

template<class T>
inline Vector4<T> Vector4<T>::operator-(const Vector4<T> &right) const {
    return Vector4<T>(e[0] - right[0], e[1] - right[1], e[2] - right[2]);
}

template<class T>
inline Vector4<T> Vector4<T>::operator*(const Vector4<T> &right) const {
    return Vector4<T>(e[0] * right[0], e[1] * right[1], e[2] * right[2]);
}

template<class T>
inline Vector4<T> Vector4<T>::operator/(const Vector4<T> &right) const {
    return Vector4<T>(e[0] / right[0], e[1] / right[1], e[2] / right[2]);
}

template<class T>
inline Vector4<T> Vector4<T>::operator/(float t) const {
    return Vector4<T>(e[0] / t, e[1] / t, e[2] / t);
}

template<class T>
inline Vector4<T> Vector4<T>::operator*(float t) const {
    return Vector4<T>(e[0] * t, e[1] * t, e[2] * t);
}

template<class T>
inline Vector4<T> operator*(float t, const Vector4<T> &right) {
    return right * t;
}

template<class T>
inline Vector4<T>& Vector4<T>::operator*=(float t) const {
    e[0] *= t; e[1] *= t; e[2] *= t;;
    return *this;
}

template<class T>
inline float Vector4<T>::length2() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
}

template<class T>
inline float Vector4<T>::length() const {
    return std::sqrt(length2());
}

template<class T>
inline Vector4<T>& Vector4<T>::norm() {
    e[0] /= length(); e[1] /= length(); e[2] /= length();
    return *this;
}

template<class T>
inline Vector4<T> Vector4<T>::normalized() const {
    return Vector4<T>(e[0] / length(), e[1] / length(), e[2] / length());
}

template<class T>
inline float Vector4<T>::dot(const Vector4<T> &right) const {
    return e[0] * right[0] + e[1] * right[1] + e[2] * right[2];
}

template<class T>
inline float Vector4<T>::dot(const Vector4<T> &left, const Vector4<T> &right) {
    return left.dot(right);
}

template<class T>
inline Vector4<T> Vector4<T>::cross(const Vector4<T> &left, const Vector4<T> &right) {
    return Vector4<T>(left[1] * right[2] - left[2] * right[1],
                      left[2] * right[0] - left[0] * right[2],
                      left[0] * right[1] - left[1] * right[0]);
}

template<class T>
inline Vector4<T> Vector4<T>::lerp(const Vector4<T> &left, const Vector4<T> &right, float t) {
    return left + (right - left) * t;
}

template<class T>
inline std::ostream& operator<<(std::ostream& out, const Vector4<T> &v) {
    out << std::format("Vec({:.3f},{:.3f},{:.3f},{:.3f})", v[0], v[1], v[2], v[3]);
    return out;
}

template<class T>
inline std::istream& operator>>(std::istream& in, Vector4<T> &v) {
    in >> v[0] >> v[1] >> v[2] >> v[3];
    return in;
}


}  // namespace lxrr