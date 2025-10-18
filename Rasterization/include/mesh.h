#pragma once
#include "vertex.h"
#include "transform.h"
#include <vector>

namespace lxrr {

class Mesh
{
public:
    Mesh() {
        vertex_list = std::make_shared<std::vector<std::shared_ptr<Vertex>>>();
    }
    ~Mesh() = default;

    std::shared_ptr<std::vector<std::shared_ptr<Vertex>>> GetVertexs() { return vertex_list; }

    void AddVertex(const vec3 &p, float u, float v, const color4 &color = Color::white, const std::shared_ptr<PhongMaterial> m=nullptr,
           const std::shared_ptr<Texture> t=nullptr);
    void AddVertex(const vec3 &p, float u, float v, const vec3 &n, const color4 &color = Color::white, const std::shared_ptr<PhongMaterial> m=nullptr,
           const std::shared_ptr<Texture> t=nullptr);
    void AddVertex(float x, float y, float z, float u, float v, const color4 &color = Color::white, const std::shared_ptr<PhongMaterial> m=nullptr,
           const std::shared_ptr<Texture> t=nullptr);
    void AddVertex(float x, float y, float z, float u, float v, const vec3 &n, const color4 &color = Color::white, const std::shared_ptr<PhongMaterial> m=nullptr,
           const std::shared_ptr<Texture> t=nullptr);
private:
    std::shared_ptr<std::vector<std::shared_ptr<Vertex>>> vertex_list;
};

} // namespace lxrr
