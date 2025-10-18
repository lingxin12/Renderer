#pragma once
#include "hittable.h"
#include "texture.hpp"

namespace lxrt {

class Material {
public:
    virtual ~Material() = default;

    virtual bool scatter(const Ray& r_in, const HitRecord& rec, color& attenuation, Ray& scattered) const {
        return false;
    }
private:
};



class Lambertian : public Material {
public:
    Lambertian(const color& albedo): tex_(std::make_shared<SolidColor>(albedo)) {}
    Lambertian(std::shared_ptr<Texture> tex): tex_(tex) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, color& attenuation, Ray& scattered) const override {
        vec3 scatter_direction = rec.normal + uniform_random_vector_on_unit_sphere();

        if (scatter_direction.x() < 1e-8 && scatter_direction.y() < 1e-8 && scatter_direction.z() < 1e-8)
            scatter_direction = rec.normal;

        scattered = Ray(rec.p, scatter_direction);
        attenuation = tex_->value(rec.u, rec.v, rec.p);
        return true;
    }
private:
    std::shared_ptr<Texture> tex_;
};



class Metal : public Material {
  public:
    Metal(const color& albedo, double fuzz) : albedo_(albedo), fuzz_(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, color& attenuation, Ray& scattered)
    const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + fuzz_ * uniform_random_vector_on_unit_sphere();
        scattered = Ray(rec.p, reflected);
        attenuation = albedo_;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

  private:
    color albedo_;
    double fuzz_;
};



class Dielectric : public Material {
  public:
    Dielectric(double refraction_index) : refraction_index_(refraction_index) {}

    bool scatter(const Ray& r_in, const HitRecord& rec, color& attenuation, Ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index_) : refraction_index_;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

          if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = Ray(rec.p, direction);
        return true;
    }

  private:
    double refraction_index_;

    static double reflectance(double cosine, double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

} // namespace lxrt