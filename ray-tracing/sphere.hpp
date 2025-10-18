#pragma once
#include "hittable.h"
#include "vec3.hpp"

namespace lxrt {

class Sphere : public Hittable {
public:
    Sphere() : center_(point3(0,0,0)), radius_(1.0), material_(nullptr) {}
    Sphere(const point3& center, double radius, std::shared_ptr<Material> material)
        : center_(center), radius_(radius), material_(material)
    {
        auto rvec = vec3(radius, radius, radius);
        bbox_ = AABB(center - rvec, center + rvec);
    }

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        vec3 oc = center_ - r.origin();
        auto a = r.direction().length2();
        auto h = dot(r.direction(), oc);
        auto c = oc.length2() - radius_*radius_;
        auto discriminant = h*h - a*c;

        if (discriminant < 0)
            return false;
        
        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (root <= tmin || tmax <= root) {
            root = (h + sqrtd) / a;
            if (root <= tmin || tmax <= root)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center_) / radius_;
        rec.set_face_normal(r, outward_normal);
        rec.material = material_;

        return true;
    }

    AABB BoundingBox() const override { return bbox_; }

private:
    point3 center_;
    double radius_;
    std::shared_ptr<Material> material_;
    AABB bbox_;
};

} // namespace lxrt