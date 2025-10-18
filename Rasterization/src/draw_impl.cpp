#include "draw_impl.h"

namespace lxrr {

void ScanlineRender::RasterizeTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, std::shared_ptr<DepthBuffer> zbuffer,
                                    RasterizationOut &frag_out,
                                    const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                                    const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                                    const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                                    const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2) {
    int v0_y = v0.pos.y();
    int v1_y = v1.pos.y();
    int v2_y = v2.pos.y();

    // std::cout << "-------------------------- RasterizeTriangle --------------------------" << std::endl;
    // std::cout << v0.pos << " " << v1.pos << " " << v2.pos << std::endl;

    if (v0_y == v1_y) {
        RasterizeTopFlatTriangle(v0, v1, v2, zbuffer, frag_out, lp0, lp1, lp2, v0, v1, v2, world_p0, world_p1, world_p2, world_n0, world_n1, world_n2, view_p0, view_p1, view_p2);
    } else if (v1_y == v2_y) {
        RasterizeBottomFlatTriangle(v0, v1, v2, zbuffer, frag_out, lp0, lp1, lp2, v0, v1, v2, world_p0, world_p1, world_p2, world_n0, world_n1, world_n2, view_p0, view_p1, view_p2);
    } else {
        // lerp vertex
        float t = ((float)v1_y - v0_y) / (v2_y - v0_y);
        float x3 = t * (v2.pos.x() - v0.pos.x()) + v0.pos.x();
        // std::cout << "t : " << t << " " <<  (v2.pos.x() - v0.pos.x()) << " " << v0.pos.x() << std::endl;
        float y3 = v1_y;
        
        Vertex v3 = Vertex::LerpVertex_NoPosXY(v0, v2, t);
        v3.pos[0] = x3;
        v3.pos[1] = y3;
        // std::cout << "v3: " << v3.pos << std::endl;

        RasterizeBottomFlatTriangle(v0, v1, v3, zbuffer, frag_out, lp0, lp1, lp2, v0, v1, v2, world_p0, world_p1, world_p2, world_n0, world_n1, world_n2, view_p0, view_p1, view_p2);
        RasterizeTopFlatTriangle(v3, v1, v2, zbuffer, frag_out, lp0, lp1, lp2, v0, v1, v2, world_p0, world_p1, world_p2, world_n0, world_n1, world_n2, view_p0, view_p1, view_p2);
    }
}

void ScanlineRender::RasterizeTopFlatTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, std::shared_ptr<DepthBuffer> zbuffer,
                                           RasterizationOut &frag_out,
                                           const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                                           const Vertex &tri_p0, const Vertex &tri_p1, const Vertex &tri_p2,
                                           const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                                           const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                                           const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2) {
    float x0 = v0.pos.x();
    float y0 = v0.pos.y();
    float x1 = v1.pos.x();
    float y1 = v1.pos.y();
    float x2 = v2.pos.x();
    float y2 = v2.pos.y();

    // std::cout << "Top triangle: " << x0 << " " << y0 << " " << x1 << " " << y1 << " " << x2 << " " << y2 << std::endl;

    for (float y = y0; y <= y2; y += 1.0f) {
        float t = (y - y0) / (y2 - y0);
        
        Vertex vl = Vertex::LerpVertex_NoPosXY(v0, v2, t);
        vl.pos[0] = t * (x2 - x0) + x0;
        vl.pos[1] = y;
        
        Vertex vr = Vertex::LerpVertex_NoPosXY(v1, v2, t);
        vr.pos[0] = t * (x2 - x1) + x1;
        vr.pos[1] = y;
        
        // std::cout << "line: " << "(" << y << ")  " << vl.pos.x() << " " << vl.pos.y() << " " << vr.pos.x() << " " << vr.pos.y() << std::endl;

        RasterizeLine(vl, vr, zbuffer, frag_out, lp0, lp1, lp2, tri_p0, tri_p1, tri_p2, world_p0, world_p1, world_p2, world_n0, world_n1, world_n2, view_p0, view_p1, view_p2);
    }
}

void ScanlineRender::RasterizeBottomFlatTriangle(const Vertex &v0, const Vertex &v1, const Vertex &v2, std::shared_ptr<DepthBuffer> zbuffer,
                                              RasterizationOut &frag_out,
                                              const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                                              const Vertex &tri_p0, const Vertex &tri_p1, const Vertex &tri_p2,
                                              const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                                              const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                                              const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2) {
    float x0 = v0.pos.x();
    float y0 = v0.pos.y();
    float x1 = v1.pos.x();
    float y1 = v1.pos.y();
    float x2 = v2.pos.x();
    float y2 = v2.pos.y();

    // std::cout << "Bottom triangle: " << x0 << " " << y0 << " " << x1 << " " << y1 << " " << x2 << " " << y2 << std::endl;

    for (float y = y0; y <= y2; y += 1.0f) {
        float t = (y - y0) / (y2 - y0);

        Vertex vl = Vertex::LerpVertex_NoPosXY(v0, v2, t);
        vl.pos[0] = t * (x2 - x0) + x0;
        vl.pos[1] = y;
        
        Vertex vr = Vertex::LerpVertex_NoPosXY(v0, v1, t);
        vr.pos[0] = t * (x1 - x0) + x0;
        vr.pos[1] = y;
        
        // std::cout << "Bottom triangle: LR line v012 : " << v0.pos << " " << v1.pos << " " << v2.pos << std::endl;
        // std::cout << "Bottom triangle: LR line : " << vl.pos << " " << vr.pos << std::endl;


        // std::cout << "line: " << "(" << y << ")  " << vl.pos.x() << " " << vl.pos.y() << " " << vr.pos.x() << " " << vr.pos.y() << std::endl;

        RasterizeLine(vl, vr, zbuffer, frag_out, lp0, lp1, lp2, tri_p0, tri_p1, tri_p2, world_p0, world_p1, world_p2, world_n0, world_n1, world_n2, view_p0, view_p1, view_p2);
    }
}

void ScanlineRender::RasterizeLine(const Vertex &v0, const Vertex &v1, std::shared_ptr<DepthBuffer> zbuffer,
                                RasterizationOut &frag_out,
                                const vec4 &lp0, const vec4 &lp1, const vec4 &lp2,
                                const Vertex &tri_p0, const Vertex &tri_p1, const Vertex &tri_p2,
                                const vec3 &world_p0, const vec3 &world_p1, const vec3 &world_p2,
                                const vec3 &world_n0, const vec3 &world_n1, const vec3 &world_n2,
                                const vec3 &view_p0, const vec3 &view_p1, const vec3 &view_p2) {
    float x0 = v0.pos.x();
    float y0 = v0.pos.y();
    float x1 = v1.pos.x();
    float y1 = v1.pos.y();

    // std::cout << "RasterizeLine : " << x0 << " " << y0 << " " << v0.pos.z() << " " << x1 << " " << y1 << " " << v1.pos.z() << " " << " --- tri: " << tri_p0.pos << " " << tri_p1.pos << " " << tri_p2.pos << std::endl;

    int dx = x1 - x0;
    int stepx = 1;

    if (dx < 0) {
        stepx = -1;
        dx = -dx;
    }

    int x = x0;
    int y = y0;

    // 重心坐标透视矫正
    float z_A = view_p0.z();
    float z_B = view_p1.z();
    float z_C = view_p2.z();

    auto barycentric_lerp = [z_A,z_B,z_C](auto _a, auto _b, auto _c, vec3 _g) {
        float z = 1.0f / (_g.x() / z_A + _g.y() / z_B + _g.z() / z_C);
        return (_a * _g.x() / z_A + _b * _g.y() / z_B + _c * _g.z() / z_C) * z;
    };

    vec3 g = CenterOfGravity(tri_p0.pos.get_3d(), tri_p1.pos.get_3d(), tri_p2.pos.get_3d(), vec2((float)x+0.5f,(float)y+0.5f));
    if (x0 == x1 && !(g.x() < 0 || g.x() > 1 || g.y() < 0 || g.y() > 1 || g.z() < 0 || g.z() > 1)) {
        // std::cout << "x0 == x1 render" << std::endl;
        // std::cout << "00 now x y z:  " << x << " " << y << " " << (std::min(v0.pos.z(), v1.pos.z())+1)/2.0 << std::endl;
        // std::cout << "Ztest " << ZTestAndWrite(x, y, (std::min(v0.pos.z(), v1.pos.z())+1)/2.0, zbuffer) << std::endl;

        if (ZTestAndWrite(x, y, (std::max(v0.pos.z(), v1.pos.z())+1)/2.0, zbuffer)) {
        // if (ZTestAndWrite(x, y, (v0.pos.z()+1)/2.0, zbuffer)) {
            // std::cout << " 00--------- WritePixel:  " << x << " " << y << " " << v0.pos.z() << " " << v1.pos.z() << std::endl;
            // std::cout << " 00--------- WritePixel:  " << x << " " << y << " " << (std::min(v0.pos.z(), v1.pos.z())+1)/2.0 << " " << zbuffer->GetWidth() << std::endl;
            vec3 g = CenterOfGravity(tri_p0.pos.get_3d(), tri_p1.pos.get_3d(), tri_p2.pos.get_3d(), vec2((float)x+0.5f,(float)y+0.5f));
            int index = (y * zbuffer->GetWidth() + x);
            (*frag_out.frag_visible_list)[index] = true;
            *((*frag_out.frag_barycentric_coord_list)[index]) = g;
            *((*frag_out.frag_color_list)[index]) = barycentric_lerp(tri_p0.color, tri_p1.color, tri_p2.color, g);
            *((*frag_out.frag_uv_list)[index]) = barycentric_lerp(tri_p0.uv, tri_p1.uv, tri_p2.uv, g);
            
            *((*frag_out.frag_world_pos_list)[index]) = barycentric_lerp(world_p0, world_p1, world_p2, g);
            *((*frag_out.frag_world_normal_list)[index]) = barycentric_lerp(world_n0, world_n1, world_n2, g);
            
            (*frag_out.frag_material_list)[index] = tri_p0.material;
            (*frag_out.frag_texture_list)[index] = tri_p0.texture;
            
            *((*frag_out.lp0_list)[index]) = lp0;
            *((*frag_out.lp1_list)[index]) = lp1;
            *((*frag_out.lp2_list)[index]) = lp2;
        }
        // std::cout << "x0 == x1 end" << std::endl;
        return;
    }

    for (int i = 0; i < dx; ++ i, x += stepx) {
        // std::cout << "now_x " << x << std::endl;
        vec3 g = CenterOfGravity(tri_p0.pos.get_3d(), tri_p1.pos.get_3d(), tri_p2.pos.get_3d(), vec2((float)x+0.5f,(float)y+0.5f));
        // if (y == 26) {
        //     std::cout << "CPU before test: " << tri_p0.pos.get_3d() << " " << tri_p1.pos.get_3d() << " " << tri_p2.pos.get_3d() << " " << x << " " << y << " " << g << std::endl;
        // }
        if (g.x() < 0 || g.x() > 1 || g.y() < 0 || g.y() > 1 || g.z() < 0 || g.z() > 1) {
            continue;
        }
        // std::cout << "g test yes " << std::endl;

        // https://zhuanlan.zhihu.com/p/144331875
        // ???????????????????? z or view_z
        float s = ((float)x - x0) / (x1 - x0);
        float t = s * v0.pos.z() / (s * v0.pos.z() + (1 - s) * v1.pos.z());
        // std::cout << "pers desify : " << s << " " << t << " " << lerp(v0.pos.z(), v1.pos.z(), t) << std::endl;
        float z = lerp(v0.pos.z(), v1.pos.z(), t);
        // float z = lerp(v0.pos.z(), v1.pos.z(), s);
        z = (z + 1) / 2.0;
        
        // std::cout << "11 now x y z:  " << x << " " << y << " " << z << std::endl;
        // std::cout << "Ztest " << ZTestAndWrite(x, y, z, zbuffer) << std::endl;

        if (ZTestAndWrite(x, y, z, zbuffer)) {
            // std::cout << " 11--------- WritePixel:  " << x << " " << y << " " << z << std::endl;
            // std::cout << "tri  : " << tri_p0.pos.get_3d() << " " << tri_p1.pos.get_3d() << " " << tri_p2.pos.get_3d() << vec2((float)x+0.5f,(float)y+0.5f) << std::endl;
            // if (y == 26) {
            //     std::cout << "CPU: y26" << std::endl;
            // }
            vec3 g = CenterOfGravity(tri_p0.pos.get_3d(), tri_p1.pos.get_3d(), tri_p2.pos.get_3d(), vec2((float)x+0.5f,(float)y+0.5f));
            
            int index = (y * zbuffer->GetWidth() + x);
            (*frag_out.frag_visible_list)[index] = true;
            *((*frag_out.frag_barycentric_coord_list)[index]) = g;
            *((*frag_out.frag_color_list)[index]) = barycentric_lerp(tri_p0.color, tri_p1.color, tri_p2.color, g);
            *((*frag_out.frag_uv_list)[index]) = barycentric_lerp(tri_p0.uv, tri_p1.uv, tri_p2.uv, g);
            // std::cout << "tri uv : " << tri_p0.uv << " " << tri_p1.uv << " " << tri_p2.uv << " " << *((*frag_out.frag_uv_list)[index]) << std::endl;

            *((*frag_out.frag_world_pos_list)[index]) = barycentric_lerp(world_p0, world_p1, world_p2, g);
            *((*frag_out.frag_world_normal_list)[index]) = barycentric_lerp(world_n0, world_n1, world_n2, g);
            // std::cout << "frag_world_normal : " << barycentric_lerp(world_n0, world_n1, world_n2, g) << std::endl;
            // exit(0);

            (*frag_out.frag_material_list)[index] = tri_p0.material;
            (*frag_out.frag_texture_list)[index] = tri_p0.texture;

            if (x == 38 && y == 29) {
                printf("CPU RS xy(38,29) lp check :: lp0(%.3f %.3f %.3f) lp1(%.3f %.3f %.3f) lp2(%.3f %.3f %.3f) xyz(%d %d %.3f)\n", lp0.x(), lp0.y(), lp0.z(), lp1.x(), lp1.y(), lp1.z(), lp2.x(), lp2.y(), lp2.z(), x, y, z);
            }

            *((*frag_out.lp0_list)[index]) = lp0;
            *((*frag_out.lp1_list)[index]) = lp1;
            *((*frag_out.lp2_list)[index]) = lp2;
        }
    }
}

float ScanlineRender::CalculateShadow(vec4 pos_light_space, double bias, std::shared_ptr<DepthBuffer> depth_buffer) {
    float w = 1.0f / pos_light_space.get_w();
    pos_light_space[0] = (pos_light_space.x() * w + 1.0f) * 0.5f * depth_buffer->GetWidth() - 1e-12f;
    pos_light_space[1] = (pos_light_space.y() * w + 1.0f) * 0.5f * depth_buffer->GetHeight() - 1e-12f;

    float depth = (pos_light_space.z() + 1.0f) / 2.0f;

    float shadow = 0.0f;
    for (int i = -4; i <= 4; i ++) {
        for (int j = -4; j <= 4; j ++) {
            float depthbuffer_depth = depth_buffer->Get(pos_light_space.x() + i, pos_light_space.y() + j);
            shadow += depth > depthbuffer_depth + bias ? 1.0f : 0.0f;
        }
    }
    shadow /= 81.0f;

    return shadow;
}

void ScanlineRender::ViewpointTransform(Vertex &vertex, unsigned int screen_width, unsigned int screen_height) {
    float w = 1.0f / vertex.pos.get_w();

    // 视口变换公式[-1,1]->[0,W]应为 x_screen = (x_ndc+1)/2*W
    // 但是我们向下取整之后需要的是[0,W-1], 下列处理省略了一个边界检查
    // vertex.pos[0] = (vertex.pos.x() * w + 1.0f) * 0.5f * (screen_width - 1.0f) + 0.5f;
    // vertex.pos[1] = (vertex.pos.y() * w + 1.0f) * 0.5f * (screen_height - 1.0f) + 0.5f;

    // 原始的公式
    vertex.pos[0] = (vertex.pos.x() * w + 1.0f) * 0.5f * screen_width - 1e-12f;
    vertex.pos[1] = (vertex.pos.y() * w + 1.0f) * 0.5f * screen_height - 1e-12f;
}

bool ScanlineRender::ZTestAndWrite(int x, int y, float depth, std::shared_ptr<DepthBuffer> zbuffer) {
    if (x >= 0 && x < zbuffer->GetWidth() && y >= 0 && y < zbuffer->GetHeight()) {
        // std::cout << "ZTestAndWrite: " << "(" << x << "," << y << ")" << "  " << zbuffer->GetWidth() << " " << zbuffer->GetHeight() << std::endl;
        // std::cout << "ZTestAndWrite: " << "(" << x << "," << y << ") ==> " << depth << " " << zbuffer->Get(y, x) << std::endl;
        if (depth <= zbuffer->Get(y, x)) {
            zbuffer->Write(y, x, depth);
            return true;
        }
    }
    return false;
}

} // namespace lxrr