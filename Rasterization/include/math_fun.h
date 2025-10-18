#pragma once
#include <cmath>
#include <chrono>
#include "vector.h"
#define PI 3.14159265358979323846f

namespace lxrr {

auto lerp(auto a, auto b, float t) -> decltype(a) {
    return a + (b - a) * t;
}

auto clamp(auto min, auto max, auto value) -> decltype(value) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline vec3 CenterOfGravity(vec3 a, vec3 b, vec3 c, vec2 p) {
    if ((a.y()-c.y())*b.x()+(c.x()-a.x())*b.y()+(a.x()*c.y()-c.x()*a.y())==0) {
        return vec3(1,0,0);
    }
    if ((a.y()-b.y())*c.x()+(b.x()-a.x())*c.y()+(a.x()*b.y()-b.x()*a.y())==0) {
        return vec3(1,0,0);
    }
    float beta = std::abs((a.y()-c.y())*p.x()+(c.x()-a.x())*p.y()+(a.x()*c.y()-c.x()*a.y())) /
        std::abs((a.y()-c.y())*b.x()+(c.x()-a.x())*b.y()+(a.x()*c.y()-c.x()*a.y()));
    float gamma = std::abs((a.y()-b.y())*p.x()+(b.x()-a.x())*p.y()+(a.x()*b.y()-b.x()*a.y())) /
        std::abs((a.y()-b.y())*c.x()+(b.x()-a.x())*c.y()+(a.x()*b.y()-b.x()*a.y()));
    // printf("---cofg--- ::  a(%.3f, %.3f, %.3f), b(%.3f, %.3f, %.3f), c(%.3f, %.3f, %.3f), p(%.3f, %.3f)\n", a.x(), a.y(), a.z(), b.x(), b.y(), b.z(), c.x(), c.y(), c.z(), p.x(), p.y());
    float alpha = 1 - beta - gamma;
    // printf("---cofg--- :: alpha: %.3f, beta: %.3f, gamma: %.3f\n", alpha, beta, gamma);
    return vec3(alpha, beta, gamma);
}

inline double calculate_time_span(std::chrono::high_resolution_clock::time_point start) {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

} // namespace lxrr