#pragma once
#include "vector.h"
#include "matrix.h"
#include "transform.h"

namespace lxrr {

class Camera
{
public:
    Camera():pos(vec3(0,0,0)), rotation(vec3(0,0,0)), scale(vec3(1,1,1)),
             object_to_world(mat4x4()), world_to_view(mat4x4()), perspective_matrix(mat4x4()) {}
    Camera(const vec3 &pos, const vec3 &rotation, const vec3 &scale):
        pos(pos), rotation(rotation), scale(scale),
        object_to_world(mat4x4()), world_to_view(mat4x4()), perspective_matrix(mat4x4()) {}
    Camera(const vec3 &pos, const vec3 &rotation, const vec3 &scale,
           const mat4x4 &o2w, const mat4x4 &w2v, const mat4x4 &v2c):
        pos(pos), rotation(rotation), scale(scale),
        object_to_world(o2w), world_to_view(w2v), perspective_matrix(v2c) {}

    ~Camera() = default;

    vec3 get_pos() const { return pos; }
    vec3 get_rotation() const { return rotation; }
    vec3 get_scale() const { return scale; }
    mat4x4 get_object_to_world() const { return object_to_world; }
    mat4x4 get_world_to_view() const { return world_to_view; }
    mat4x4 get_perspective_matrix() const { return perspective_matrix; }

    // https://www.zhyingkun.com/perspective/perspective/
    static mat4x4 LookAt(const vec3 &pos, const vec3 &look_at, const vec3 &up_axis=vec3(0,1,0));
    static mat4x4 Perspective(float fovy, float aspect, float near, float far);
    // f and t are signed coordinate, instead of far and near
    // But the depth is positive after using View Matrix, So the n and f is not negative even if camera is oriented towards the -z axis
    static mat4x4 Perspective(float l, float r, float n, float f, float t, float b);
    static mat4x4 Orthographic(float l, float r, float n, float f, float t, float b);


private:
    vec3 pos;
    vec3 rotation;
    vec3 scale;
    mat4x4 object_to_world;
    mat4x4 world_to_view;
    mat4x4 perspective_matrix;
};

} // namespace lxrr