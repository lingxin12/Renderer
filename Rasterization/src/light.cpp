#include "light.h"

namespace lxrr {

mat4x4 DirectionLight::LookAt(const vec3 &up_axis) {
    vec3 look_dir = dir.normalized();
    vec3 right_dir = vec3::cross(up_axis, look_dir).normalized();
    vec3 up_dir = vec3::cross(look_dir, right_dir);

    mat4x4 m;
    m(0,0) = right_dir.x(); m(1,0) = up_dir.x(); m(2,0) = look_dir.x(); m(3,0) = 0.0f;
    m(0,1) = right_dir.y(); m(1,1) = up_dir.y(); m(2,1) = look_dir.y(); m(3,1) = 0.0f;
    m(0,2) = right_dir.z(); m(1,2) = up_dir.z(); m(2,2) = look_dir.z(); m(3,2) = 0.0f;
    m(0,3) = -pos.x(); m(1,3) = -pos.y(); m(2,3) = -pos.z(); m(3,3) = 1.0f;
        

    return m;
}

} // namespace lxrr