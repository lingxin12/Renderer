#include "mesh.h"

namespace lxrr {

void Mesh::AddVertex(const vec3 &p, float u, float v, const color4 &color, const std::shared_ptr<PhongMaterial> m,
           const std::shared_ptr<Texture> t) {
    Vertex vertex(vec4(p), color, vec2(u,v), m, t);
    vertex_list->push_back(std::make_shared<Vertex>(vertex));
}

void Mesh::AddVertex(const vec3 &p, float u, float v, const vec3 &n, const color4 &color, const std::shared_ptr<PhongMaterial> m,
           const std::shared_ptr<Texture> t) {
    Vertex vertex(vec4(p), n, color, vec2(u,v), m ,t);
    vertex_list->push_back(std::make_shared<Vertex>(vertex));
}

void Mesh::AddVertex(float x, float y, float z, float u, float v, const color4 &color, const std::shared_ptr<PhongMaterial> m,
           const std::shared_ptr<Texture> t) {
    AddVertex(vec3(x,y,z), u, v, color, m ,t);
}

void Mesh::AddVertex(float x, float y, float z, float u, float v, const vec3 &n, const color4 &color, const std::shared_ptr<PhongMaterial> m,
           const std::shared_ptr<Texture> t) {
    AddVertex(vec3(x,y,z), u, v, n, color, m ,t);
}

} // namespace lxrr