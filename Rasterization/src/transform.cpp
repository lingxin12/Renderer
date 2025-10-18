#include "transform.h"

namespace lxrr {

mat4x4 Transform::translate(const vec3 &v) {
    mat4x4 m;
    m(0,3) = v[0];
    m(1,3) = v[1];
    m(2,3) = v[2];
    return m;
}

mat3x3 Transform::rotate_x(float angle) {
    mat3x3 m;
    float radian = angle * PI / 180.0f;
    float cos_value = cos(radian);
    float sin_value = sin(radian);
    m(1,1) = cos_value;
    m(1,2) = -sin_value;
    m(2,1) = sin_value;
    m(2,2) = cos_value;
    return m;
}

mat3x3 Transform::rotate_y(float angle) {
    mat3x3 m;
    float radian = angle * PI / 180.0f;
    float cos_value = cos(radian);
    float sin_value = sin(radian);
    m(0,0) = cos_value;
    m(0,2) = -sin_value;
    m(2,0) = sin_value;
    m(2,2) = cos_value;
    return m;
}

mat3x3 Transform::rotate_z(float angle) {
    mat3x3 m;
    float radian = angle * PI / 180.0f;
    float cos_value = cos(radian);
    float sin_value = sin(radian);
    m(1,1) = cos_value;
    m(1,2) = -sin_value;
    m(2,1) = sin_value;
    m(2,2) = cos_value;
    return m;
}

mat3x3 Transform::rotate(const vec3 &rot_angle) {
    auto rot_x = rotate_x(rot_angle[0]);
    auto rot_y = rotate_y(rot_angle[1]);
    auto rot_z = rotate_z(rot_angle[2]);
    return rot_z * rot_y * rot_x;
}

mat3x3 Transform::scale(const vec3 &s) {
    mat3x3 m;
    m(0,0) = s[0];
    m(1,1) = s[1];
    m(2,2) = s[2];
    return m;
}


} // namespace lxrr