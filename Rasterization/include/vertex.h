#pragma once
#include <format>
#include <memory>
#include "vector.h"
#include "matrix.h"
#include "math_fun.h"
#include "color.h"
#include "material.h"
#include "texture.h"


namespace lxrr {

class Vertex {
public:
    Vertex() {
        material = MaterialLibrary::GetInstance().GetMaterial("default");
        texture = std::make_shared<Texture>();
    }
    ~Vertex() {}
    Vertex(const vec4 &p, const color4 &c, const vec2 &tex_coord,
           const std::shared_ptr<PhongMaterial> m=nullptr,
           const std::shared_ptr<Texture> t=nullptr):
        pos(p), normal(vec3()), color(c), uv(tex_coord), material(m), texture(t) { }
    Vertex(const vec4 &p, const vec3 &n, const color4 &c, const vec2 &tex_coord,
           const std::shared_ptr<PhongMaterial> m=nullptr,
           const std::shared_ptr<Texture> t=nullptr):
        pos(p), normal(n), color(c), uv(tex_coord), material(m), texture(t) { }

    static Vertex LerpVertex(const Vertex &left, const Vertex &right, float t);
    static Vertex LerpVertex_NoPosXY(const Vertex &left, const Vertex &right, float t);
    Vertex& operator*(const mat4x4 &m);
    friend std::ostream& operator<<(std::ostream &out, const Vertex &v);

    vec4 pos;
    vec3 normal;
    color4 color;
    vec2 uv;
    std::shared_ptr<PhongMaterial> material;
    std::shared_ptr<Texture> texture;
};

} // namespace lxrr