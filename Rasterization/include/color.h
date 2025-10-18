#pragma once
#include "vector.h"

namespace lxrr {

using color3 = Vector3<float>;
using color4 = Vector4<float>;

class Color {
public:
    Color() = default;
    ~Color() = default;

    template<typename T>
    static T lerp(T a, T b, float t) {
        return T(lerp(a.x(), b.x(), t), lerp(a.y(), b.y(), t), lerp(a.z(), b.z(), t));
    }

    inline static const color4 white = color4(1.0, 1.0, 1.0, 1.0);
    inline static const color4 red = color4(1.0, 0.0, 0.0, 1.0);
    inline static const color4 green = color4(0.0, 1.0, 0.0, 1.0);
    inline static const color4 blue = color4(0.0, 0.0, 1.0, 1.0);
    inline static const color4 black = color4(0.0, 0.0, 0.0, 1.0);
};

} // namespace lxrr