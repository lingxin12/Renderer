#pragma once
#include "vector.h"
#include "color.h"
#include "light.h"
#include "material.h"

namespace lxrr {

// https://zhuanlan.zhihu.com/p/442023993
// Direction Light
color4 BlinnPhongModel(const vec3 &pos, const vec3 &normal, const vec3 &view_pos,
                       const DirectionLight &direction_light, const PhongMaterial &material);

// Point Light
color4 BlinnPhongModel(const vec3 &pos, const vec3 &normal, const vec3 &view_pos,
                       const PointLight &direction_light, const PhongMaterial &material);

} // namespace lxrr