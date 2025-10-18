#pragma once
#include "vec3.hpp"

#include "cuda.h"
#include "cuda_runtime.h"
#include "vector_types.h"

#include "auxiliary_cuda.h"

namespace lxrt {

class Ray {
public:
    __host__ Ray(): orig(point3(0,0,0)), dir(vec3(0,0,0)) {}
    __host__ Ray(const point3& origin, const vec3& direction): orig(origin), dir(direction) {}

    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }

    point3 at(double t) const {
        return orig + t * dir;
    }


private:
    point3 orig;
    vec3 dir;
};

} // namespace lxrt