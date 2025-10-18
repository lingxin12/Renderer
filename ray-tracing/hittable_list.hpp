#pragma once
#include "hittable.h"

#include <memory>
#include <vector>

namespace lxrt {

class HittableList : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;

    HittableList() = default;
    HittableList(std::shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(std::shared_ptr<Hittable> object) {
        objects.push_back(object);
        bbox_ = AABB(bbox_, object->BoundingBox());
    }

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = tmax;

        for (const auto& object : objects) {
            if (object->hit(r, tmin, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    AABB BoundingBox() const override { return bbox_; }

private:
    AABB bbox_;
};

} // namespace lxrt