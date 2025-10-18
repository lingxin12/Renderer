#pragma once
#include <memory>
#include "ray.hpp"
#include "aabb.hpp"

namespace lxrt {

class Material;

struct HitRecord {
    point3 p;
    vec3 normal;
    std::shared_ptr<Material> material;
    double t;
    double u;
    double v;
    bool front_face;

    void set_face_normal(const Ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;

    virtual bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const = 0;

    virtual AABB BoundingBox() const = 0;
};

}