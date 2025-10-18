#pragma once
#include "memory"
#include "vec3.hpp"

namespace lxrt {

class Texture {
public:
    virtual ~Texture() = default;
    virtual color value(double u, double v, const point3& p) const = 0;
};

class SolidColor : public Texture {
public:
    SolidColor() = default;
    SolidColor(color c) : albedo_(c) {}
    SolidColor(double r, double g, double b) : albedo_(color(r, g, b)) {}

    color value(double u, double v, const point3& p) const override {
        return albedo_;
    }

private:
    color albedo_;
};


class CheckerTexture : public Texture {
  public:
    CheckerTexture(double scale, std::shared_ptr<Texture> even, std::shared_ptr<Texture> odd)
        : inv_scale_(1.0 / scale), even_(even), odd_(odd) {}

    CheckerTexture(double scale, const color& c1, const color& c2)
    : CheckerTexture(scale,
        std::static_pointer_cast<Texture>(std::make_shared<SolidColor>(c1)),
        std::static_pointer_cast<Texture>(std::make_shared<SolidColor>(c2))) {}

    color value(double u, double v, const point3& p) const override {
        auto xInteger = int(std::floor(inv_scale_ * p.x()));
        auto yInteger = int(std::floor(inv_scale_ * p.y()));
        auto zInteger = int(std::floor(inv_scale_ * p.z()));

        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even_->value(u, v, p) : odd_->value(u, v, p);
    }

  private:
    double inv_scale_;
    std::shared_ptr<Texture> even_;
    std::shared_ptr<Texture> odd_;
};

} // namespace lxrt