#include <cmath>
#include "camera.h"
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

namespace lxrr {

mat4x4 Camera::LookAt(const vec3 &pos, const vec3 &look_at, const vec3 &up_axis) {
    vec3 look_dir = look_at.normalized();
    vec3 right_dir = vec3::cross(up_axis, look_dir);
    vec3 up_dir = vec3::cross(look_dir, right_dir);

    mat4x4 m;
    m(0,0) = right_dir.x(); m(1,0) = up_dir.x(); m(2,0) = look_dir.x(); m(3,0) = 0.0f;
    m(0,1) = right_dir.y(); m(1,1) = up_dir.y(); m(2,1) = look_dir.y(); m(3,1) = 0.0f;
    m(0,2) = right_dir.z(); m(1,2) = up_dir.z(); m(2,2) = look_dir.z(); m(3,2) = 0.0f;
    m(0,3) = -pos.x(); m(1,3) = -pos.y(); m(2,3) = -pos.z(); m(3,3) = 1.0f;

    return m;
}

mat4x4 Camera::Perspective(float fovy, float aspect, float near, float far) {
    float tanfov = std::tan(0.5f * fovy * PI / 180.0f);

    mat4x4 proj;
    proj(0,0) = 1/(tanfov*aspect);
    proj(1,1) = 1/tanfov;
    proj(2,2) = (far+near)/(far-near);
    proj(2,3) = -(2.0*far*near)/(far-near);
    proj(3,2) = 1.0f;
    proj(3,3) = 0;

    return proj;
}

mat4x4 Camera::Perspective(float l, float r, float n, float f, float t, float b) {
    mat4x4 proj;
    proj(0,0) = 2*n /(r-l); proj(0,1) = 0;         proj(0,2) = 0;           proj(0,3) = 0;
    proj(1,0) = 0;          proj(1,1) = 2*n/(t-b); proj(1,2) = 0;           proj(1,3) = 0;
    proj(2,0) = 0;          proj(2,1) = 0;         proj(2,2) = (n+f)/(f-n); proj(2,3) = -2*n*f/(f-n);
    proj(3,0) = 0;          proj(3,1) = 0;         proj(3,2) = 1;           proj(3,3) = 0;
    
    return proj;
}

mat4x4 Camera::Orthographic(float l, float r, float n, float f, float t, float b) {
    mat4x4 ortho;

    ortho(0,0) = 2/(r-l); ortho(0,1) = 0;       ortho(0,2) = 0;       ortho(0,3) = -(r+l)/(r-l);
    ortho(1,0) = 0;       ortho(1,1) = 2/(t-b); ortho(1,2) = 0;       ortho(1,3) = -(t+b)/(t-b);
    ortho(2,0) = 0;       ortho(2,1) = 0;       ortho(2,2) = 2/(f-n); ortho(2,3) = -(f+n)/(f-n);  /////// 
    ortho(3,0) = 0;       ortho(3,1) = 0;       ortho(3,2) = 0;       ortho(3,3) = 1;

    return ortho;
}

} // namespace lxrr