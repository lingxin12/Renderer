#pragma once
#include "aabb.hpp"
#include "hittable.h"
#include "hittable_list.hpp"

#include <algorithm>

namespace lxrt {

class BVHNode : public Hittable {
public:
    BVHNode() = default;
    BVHNode(HittableList list) : BVHNode(list.objects, 0, list.objects.size()) {}
    BVHNode(std::vector<std::shared_ptr<Hittable>>& objects, size_t start, size_t end) {

        bbox_ = AABB(vec3(-kinfinity, -kinfinity, -kinfinity), vec3(kinfinity, kinfinity, kinfinity));
        for (size_t i = start; i < end; ++i)
            bbox_ = AABB(bbox_, objects[i]->BoundingBox());

        int axis = bbox_.longest_axis();

        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;
        
        size_t object_span = end - start;

        if (object_span == 1) {
            left_ = right_ = objects[start];
        } else if (object_span == 2) {
            left_ = objects[start];
            right_ = objects[start + 1];
        } else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);

            auto mid = start + object_span / 2;
            left_ = std::make_shared<BVHNode>(objects, start, mid);
            right_ = std::make_shared<BVHNode>(objects, mid, end);
        }
    }

    bool hit(const Ray& r, double tmin, double tmax, HitRecord& rec) const override {
        if (!bbox_.hit(r, tmin, tmax))
            return false;
        
        bool hit_left = left_->hit(r, tmin, tmax, rec);
        bool hit_right = right_->hit(r, tmin, hit_left ? rec.t : tmax, rec);

        return hit_left || hit_right;
    }

    AABB BoundingBox() const override {
        return bbox_;
    }

    std::shared_ptr<Hittable> left() const {
        return left_;
    }

    std::shared_ptr<Hittable> right() const {
        return right_;
    }

private:
    std::shared_ptr<Hittable> left_;
    std::shared_ptr<Hittable> right_;
    AABB bbox_;

    static bool box_x_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
        return a->BoundingBox().min().x() < b->BoundingBox().min().x();
    }

    static bool box_y_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
        return a->BoundingBox().min().y() < b->BoundingBox().min().y();
    }

    static bool box_z_compare(const std::shared_ptr<Hittable>& a, const std::shared_ptr<Hittable>& b) {
        return a->BoundingBox().min().z() < b->BoundingBox().min().z();
    }
};

} // namespace lxrt