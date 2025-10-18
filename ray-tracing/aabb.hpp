#pragma once
#include "vec3.hpp"
#include "ray.hpp"

namespace lxrt {

class AABB {
public:
    AABB() = default;

    AABB(double x0, double x1, double y0, double y1, double z0, double z1): min_(vec3(x0, y0, z0)), max_(vec3(x1, y1, z1)) {}

    AABB(const vec3& min, const vec3& max): min_(min), max_(max) {}

    AABB(const AABB& a, const AABB& b) {
        min_ = vec3(fmin(a.min_.x(), b.min_.x()), fmin(a.min_.y(), b.min_.y()), fmin(a.min_.z(), b.min_.z()));
        max_ = vec3(fmax(a.max_.x(), b.max_.x()), fmax(a.max_.y(), b.max_.y()), fmax(a.max_.z(), b.max_.z()));
    }

    bool hit(const Ray& r, double tmin, double tmax) const {
        const point3& origin = r.origin();
        const vec3& direction = r.direction();

        for (int axis = 0; axis < 3; ++axis) {
            double ax_min = min_[axis];
            double ax_max = max_[axis];
            
            auto t0 = (ax_min - origin[axis]) / direction[axis];
            auto t1 = (ax_max - origin[axis]) / direction[axis];

            if (t0 < t1) {
                if (t0 > tmin) tmin = t0;
                if (t1 < tmax) tmax = t1;
            } else {
                if (t1 > tmin) tmin = t1;
                if (t0 < tmax) tmax = t0;
            }

            if (tmax <= tmin)
                return false;
        }

        return true;
    }

    int longest_axis() const {
        vec3 span = max_ - min_;
        if (span.x() > span.y() && span.x() > span.z()) return 0;
        else if (span.y() > span.x() && span.y() > span.z()) return 1;
        else return 2;
    }

    vec3 min() const { return min_; }
    vec3 max() const { return max_; }

private:
    vec3 min_, max_;
};

} // namespace lxrt