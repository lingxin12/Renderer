#pragma once
#include <algorithm>
#include <functional>
#include "material.h"
#include "vector.h"
#include "vertex.h"
#include "shader_data.h"

namespace lxrr {

class ScanlineRender {
public:

    static void RasterizeTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, std::shared_ptr<DepthBuffer> zbuffer,
                            RasterizationOut &frag_out,
                            const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                            const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                            const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                            const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2);

    static void RasterizeTopFlatTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, std::shared_ptr<DepthBuffer> zbuffer,
                                    RasterizationOut &frag_out,
                                    const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                                    const Vertex &tri_p0, const Vertex &tri_p1, const Vertex &tri_p2,
                                    const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                                    const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                                    const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2);

    static void RasterizeBottomFlatTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, std::shared_ptr<DepthBuffer> zbuffer,
                                        RasterizationOut &frag_out,
                                        const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                                        const Vertex &tri_p0, const Vertex &tri_p1, const Vertex &tri_p2,
                                        const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                                        const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                                        const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2);

    static void RasterizeLine(const Vertex &v0, const Vertex &v1, std::shared_ptr<DepthBuffer> zbuffer,
                        RasterizationOut &frag_out,
                        const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                        const Vertex &tri_p0, const Vertex &tri_p1, const Vertex &tri_p2,
                        const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                        const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                        const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2);

    // void WritePixel(int x, int y, const color4 &color);

    static float CalculateShadow(vec4 pos_light_space, double bias, std::shared_ptr<DepthBuffer> depth_buffer);
    static void ViewpointTransform(Vertex &vertex, unsigned int screen_width, unsigned int screen_height);
    static bool ZTestAndWrite(int x, int y, float depth, std::shared_ptr<DepthBuffer> zbuffer);


private:
    ScanlineRender() = default;
};

} // namespace lxrr