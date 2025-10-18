#pragma once
#include "vector.h"
#include "matrix.h"
#include "color.h"

namespace lxrr {

class DirectionLight {
public:
    DirectionLight():pos(vec3(0,0,0)), intensity(1.0), color(Color::white), ka(0.1), kd(0.8), ks(0.2) {
        this->dir = vec3(1,0,0).normalized();
    }
    DirectionLight(const vec3 &pos, const vec3 &dir, float intensity=1, const color4 &color=Color::white, float ka=0.1, float kd=0.8, float ks=0.2):
        pos(pos), intensity(intensity), color(color), ka(ka), kd(kd), ks(ks) {
            this->dir = dir.normalized();
        }
    ~DirectionLight() = default;

    vec3 get_pos() const { return pos; }
    vec3 get_dir() const { return dir; }
    float get_intensity() const { return intensity; }
    color4 get_color() const { return color; }
    float get_ka() const { return ka; }
    float get_kd() const { return kd; }
    float get_ks() const { return ks; }

    mat4x4 LookAt(const vec3 &up_axis = vec3(0,1,0));


private:
    vec3 pos;
    vec3 dir;
    float intensity;
    color4 color;

    float ka, kd, ks;
};

class PointLight {
public:
    PointLight():pos(vec3(0,0,0)), intensity(1.0), color(Color::white), ka(0.1), kd(0.8), ks(0.2) {}
    PointLight(const vec3 &pos, float intensity=1, const color4 &color=Color::white, float ka=0.1, float kd=0.8, float ks=0.2):pos(pos), intensity(intensity), color(color), ka(0.1), kd(0.8), ks(0.2) {}
    ~PointLight() = default;

    vec3 get_pos() const { return pos; }
    float get_intensity() const { return intensity; }
    color4 get_color() const { return color; }
    float get_ka() const { return ka; }
    float get_kd() const { return kd; }
    float get_ks() const { return ks; }

private:
    vec3 pos;
    float intensity;
    color4 color;

    float ka, kd, ks;
};

} // namespace lxrr