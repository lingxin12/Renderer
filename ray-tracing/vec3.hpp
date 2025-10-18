#pragma once
#include <iostream>
#include <cmath>
#include <array>
#include "common.h"
#include "vector_types.h"

namespace lxrt {

template <typename T, size_t N>
class vectorN {
public:
    vectorN() = default;

    template <typename... Args>
    vectorN(Args... args) : e{args...} {
        static_assert(sizeof...(args) == N, "Vector initialization error");
    }

    T x() const { return e[0]; }
    T y() const { return e[1]; }
    T z() const { return e[2]; }
    T w() const { return e[3]; }
    vectorN operator-() const {
        return [this]<size_t... Indices>(std::index_sequence<Indices...>) -> vectorN {
            return vectorN(-e[Indices]...);
        }(std::make_index_sequence<N>{});
    }

    T operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i) { return e[i]; }

    vectorN& operator+=(const vectorN& v) { for(auto i=0;i<N;++i) e[i]+=v[i]; return *this; }
    vectorN& operator-=(const vectorN& v) { *this+=(-v); return *this; }
    vectorN& operator*=(double t) { for(auto i=0;i<N;++i) e[i]*=t; return *this; }
    vectorN& operator/=(double t) { return *this*=1/t; }

    double length() const { return std::sqrt(length2()); }
    double length2() const { double sum=0; for(auto n: e) sum+=n*n; return sum; }

    void unit() { *this /= length(); }

    static vectorN random() {
        vectorN v;
        for(auto i=0;i<N;++i) v[i] = random_double();
        return v;
    }

    static vectorN random(double min, double max) {
        vectorN v;
        for(auto i=0;i<N;++i) v[i] = random_double(min, max);
        return v;
    }

    
    float3 get_float3() const {
        return float3{static_cast<float>(e[0]), static_cast<float>(e[1]), static_cast<float>(e[2])};
    }

    float4 get_float4() const {
        return float4{static_cast<float>(e[0]), static_cast<float>(e[1]), static_cast<float>(e[2]), static_cast<float>(e[3])};
    }

private:
    std::array<T, N> e;
};

using vec3 = vectorN<double, 3>;
using point3 = vec3;
using color = vec3;
using vec4 = vectorN<double, 4>;



// Vector Utility Functions
inline std::ostream& operator<<(std::ostream &out, const vec3& v) {
    return out << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
}

inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u[0]+v[0], u[1]+v[1], u[2]+v[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u[0]-v[0], u[1]-v[1], u[2]-v[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u[0]*v[0], u[1]*v[1], u[2]*v[2]);
}

inline vec3 operator*(double t, const vec3& v) {
    return vec3(t*v[0], t*v[1], t*v[2]);
}

inline vec3 operator*(const vec3& v, double t) {
    return t*v;
}

inline vec3 operator/(vec3 v, double t) {
    return (1/t)*v;
}

inline double dot(const vec3& u, const vec3& v) {
    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u[1]*v[2] - u[2]*v[1],
                u[2]*v[0] - u[0]*v[2],
                u[0]*v[1] - u[1]*v[0]);
}

inline vec3 unit_vector(const vec3& v) {
    return v/v.length();
}

inline vec3 uniform_random_vector_on_unit_sphere() {
    while (true) {
        auto p = vec3::random(-1, 1);
        if (1e-160 <= p.length2() && p.length2() <= 1)
            return unit_vector(p);
    }
}

inline vec3 uniform_random_vector_on_hemisphere(const vec3& normal) {
    vec3 p = uniform_random_vector_on_unit_sphere();
    if (dot(p, normal) > 0.0)
        return p;
    else
        return -p;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2*dot(v,n)*n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length2())) * n;
    return r_out_perp + r_out_parallel;
}

} // namespace lxrt