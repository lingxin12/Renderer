#include "vertex.h"

namespace lxrr {

Vertex Vertex::LerpVertex(const Vertex &left, const Vertex &right, float t) {
    return Vertex(vec4(lerp(left.pos.get_3d(), right.pos.get_3d(), t)),
                  lerp(left.normal, right.normal, t),
                  lerp(left.color, right.color, t),
                  lerp(left.uv, right.uv, t),
                  left.material,
                  left.texture);
}

Vertex Vertex::LerpVertex_NoPosXY(const Vertex &left, const Vertex &right, float t) {
    return Vertex(vec4(0.0f, 0.0f, lerp(left.pos.z(), right.pos.z(), t)),
                  lerp(left.normal, right.normal, t),
                  lerp(left.color, right.color, t),
                  lerp(left.uv, right.uv, t),
                  left.material,
                  left.texture);
}

Vertex& Vertex::operator*(const mat4x4 &m) {
    pos = m * pos;
    normal = m.get_3x3() * normal;  // Only rotate, so ((M)^(-1))^T = M
    return *this;
}

std::ostream& operator<<(std::ostream &out, const Vertex &v) {
    out << "Vertex(\n"
        << "\tpos: " << v.pos << "\n"
        << "\tnormal: " << v.normal << "\n"
        << "\tcolor: " << v.color << "\n"
        << "\tuv: " << v.uv << "\n";
    return out;
}

} // namespace lxrr