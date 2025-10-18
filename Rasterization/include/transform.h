#pragma once
#include "matrix.h"
#include "vector.h"
#include "math_fun.h"

namespace lxrr {

class Transform {
public:
    Transform() {}

    // https://zhuanlan.zhihu.com/p/183973440
    static mat4x4 translate(const vec3 &v);
    static mat3x3 rotate_x(float angle);
    static mat3x3 rotate_y(float angle);
    static mat3x3 rotate_z(float angle);
    static mat3x3 rotate(const vec3 &rot_angle);
    static mat3x3 scale(const vec3 &s);
    
};

};