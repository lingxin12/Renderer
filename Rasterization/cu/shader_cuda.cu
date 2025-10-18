#include <cstdio>
#include <unordered_map>

#include "cuda.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "vector_types.h"

#include "cooperative_groups.h"
#include "cooperative_groups/reduce.h"
namespace cg = cooperative_groups;

#include "auxiliary_cuda.h"
#include "shader_cuda.h"

namespace lxrr_cuda {

inline void Matrix4x4ToFloatPointer(const lxrr::mat4x4 &m, float *vf) {
    for (int i = 0; i < 4; i ++) {
        for (int j = 0; j < 4; j ++) {
            vf[i * 4 + j] = m(i,j);
        }
    }
}

inline void Vector3ToFloatPointer(const lxrr::vec3 &v, float3 &vf) {
    vf.x = v.x(); vf.y = v.y(); vf.z = v.z();
}

inline void Vector4ToFloatPointer(const lxrr::vec4 &v, float4 &vf) {
    vf.x = v.x(); vf.y = v.y(); vf.z = v.z(); vf.w = v.get_w();
}

inline void FloatPointerToVector3(const float3 vf, lxrr::vec3 &v) {
    v[0] = vf.x; v[1] = vf.y; v[2] = vf.z;
}

inline void FloatPointerToVector4(const float4 vf, lxrr::vec4 &v) {
    v[0] = vf.x; v[1] = vf.y; v[2] = vf.z; v[3] = vf.w;
}


__global__ void CudaPhongShaderVS_kernel(
    uint32_t N,
    const float *M,
    const float *V,
    const float *P,
    const float *light_V,
    const float *light_P,
    const float4 *vertex_pos,
    const float3 *vertex_normal,
    const float4 *light_vertex_pos,
    float4 *result_vertex_pos,
    float4 *result_light_vertex_pos,
    float3 *result_world_pos,
    float3 *result_world_normal,
    float3 *result_view_pos) {
    
    auto block = cg::this_thread_block();
    uint32_t idx = block.group_index().x * BLOCK_X + block.thread_index().x;
    
    if (idx >= N) {
        return;
    }

    auto v_pos = vertex_pos[idx];
    auto v_normal = vertex_normal[idx];
    auto lp = light_vertex_pos[idx];
    
    lp = TransformVector4x4(M, lp);
    lp = TransformVector4x4(light_V, lp);
    lp = TransformVector4x4(light_P, lp);
    result_light_vertex_pos[idx] = lp;

    v_pos = TransformVector4x4(M, v_pos);
    result_world_pos[idx] = make_float3(v_pos.x, v_pos.y, v_pos.z);
    v_pos = TransformVector4x4(V, v_pos);
    result_view_pos[idx] = make_float3(v_pos.x, v_pos.y, v_pos.z);
    v_pos = TransformVector4x4(P, v_pos);
    v_pos = StandardizationVector(v_pos);
    result_vertex_pos[idx] = v_pos;

    v_normal = TransformVector3x3(M, v_normal);
    result_world_normal[idx] = v_normal;
}


// Get line
__global__ void CudaPhongShaderRS_pre_kernel(
    uint32_t N, uint32_t screen_width, uint32_t screen_height,
    const float4 *vertex_pos,
    float4 *left_point,
    float4 *right_point,
    uint32_t *line_count) {

    auto block = cg::this_thread_block();
    uint32_t idx = block.group_index().x * BLOCK_X + block.thread_index().x;
    if (idx >= N / 3) {
        return;
    }

    auto left_point_list = left_point + idx * screen_height;
    auto right_point_list = right_point + idx * screen_height;
    // printf("left and right _point: %d\n", idx * screen_height);
    uint32_t now_line_count = 0;

    auto v0 = vertex_pos[idx * 3 + 0];
    auto v1 = vertex_pos[idx * 3 + 1];
    auto v2 = vertex_pos[idx * 3 + 2];
    // printf("ViewpointTransform front %f %f %f\n", v0.y, v1.y, v2.y);
    ViewpointTransform(v0, screen_width, screen_height);
    ViewpointTransform(v1, screen_width, screen_height);
    ViewpointTransform(v2, screen_width, screen_height);
    // printf("ViewpointTransform back %f %f %f\n", v0.y, v1.y, v2.y);

    if (((int)v0.y == (int)v1.y && fabsf(v0.y - v2.y) <= 1) ||
        ((int)v1.y == (int)v2.y && fabsf(v2.y - v0.y) <= 1) ||
        ((int)v0.y == (int)v2.y && fabsf(v1.y - v2.y) <= 1)) {
        return;
    }

    // Triangle
    int v0_y = (int)v0.y;
    int v1_y = (int)v1.y;
    int v2_y = (int)v2.y;

    if (v0_y == v1_y) {
        float x0 = v0.x, y0 = v0.y, z0 = v0.z;
        float x1 = v1.x, y1 = v1.y, z1 = v1.z;
        float x2 = v2.x, y2 = v2.y, z2 = v2.z;

        int y_len = (int)(y2 - y0) + 1;
        float y = y0;
        // printf("y_len: %d %f %f %f\n", y_len, v0.y, v1.y, v2.y);
        for (int _ = 0; _ < y_len; _ ++) {
            float t = (y - y0) / (y2 - y0);
            
            float4 vl = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
            vl.x = t * (x2 - x0) + x0;
            vl.y = y;
            vl.z = t * (z2 - z0) + z0;

            float4 vr = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
            vr.x = t * (x2 - x1) + x1;
            vr.y = y;
            vr.z = t * (z2 - z1) + z1;

            left_point_list[now_line_count] = vl;
            right_point_list[now_line_count] = vr;
            now_line_count ++;
            // printf("v0_y == v1_y : %d\n", now_line_count);
            // printf("tri(%.3f, %.3f, %.3f)  :  vlr (%.3f %.3f) -- (%.3f %.3f)\n", v0.x, v1.x, v2.x, vl.x, vl.y, vr.x, vr.y);
            
            y += 1.0f;
        }
    } else if (v1_y == v2_y) {
        float x0 = v0.x, y0 = v0.y, z0 = v0.z;
        float x1 = v1.x, y1 = v1.y, z1 = v1.z;
        float x2 = v2.x, y2 = v2.y, z2 = v2.z;

        int y_len = (int)(y2 - y0) + 1;
        float y = y0;
        for (int _ = 0; _ < y_len; _ ++) {
            float t = (y - y0) / (y2 - y0);
            
            float4 vl = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
            vl.x = t * (x2 - x0) + x0;
            vl.y = y;
            vl.z = t * (z2 - z0) + z0;

            float4 vr = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
            vr.x = t * (x1 - x0) + x0;
            vr.y = y;
            vr.z = t * (z1 - z0) + z0;

            left_point_list[now_line_count] = vl;
            right_point_list[now_line_count] = vr;
            now_line_count ++;
            // printf("v1_y == v2_y : %d\n", now_line_count);
            // printf("tri(%.3f, %.3f, %.3f)  :  vlr (%.3f %.3f) -- (%.3f %.3f)\n", v0.x, v1.x, v2.x, vl.x, vl.y, vr.x, vr.y);
            
            y += 1.0f;
        }

    } else {
        float s = ((float)v1_y - v0_y) / (v2_y - v0_y);
        float x3 = s * (v2.x - v0.x) + v0.x;
        float y3 = (float)v1_y;
        float z3 = s * (v2.z - v0.z) + v0.z;

        // Bottom Flat Triangle (v0, v1, v3)
        {
            float x0 = v0.x, y0 = v0.y, z0 = v0.z;
            float x1 = v1.x, y1 = v1.y, z1 = v1.z;
            float x2 = x3, y2 = y3, z2 = z3;

            int y_len = (int)(y2 - y0) + 1;
            float y = y0;
            for (int _ = 0; _ < y_len; _ ++) {
                float t = (y - y0) / (y2 - y0);
                
                float4 vl = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
                vl.x = t * (x2 - x0) + x0;
                vl.y = y;
                vl.z = t * (z2 - z0) + z0;

                float4 vr = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
                vr.x = t * (x1 - x0) + x0;
                vr.y = y;
                vr.z = t * (z1 - z0) + z0;

                left_point_list[now_line_count] = vl;
                right_point_list[now_line_count] = vr;
                now_line_count ++;
                // printf("bbb : %d\n", now_line_count);
                // printf("tri(%.3f, %.3f, %.3f)  :  vlr (%.3f %.3f) -- (%.3f %.3f)\n", v0.x, v1.x, v2.x, vl.x, vl.y, vr.x, vr.y);
                
                y += 1.0f;
            }
        }

        // Top Flat Triangle (v3, v1, v2)
        {
            float x0 = x3, y0 = y3, z0 = z3;
            float x1 = v1.x, y1 = v1.y, z1 = v1.z;
            float x2 = v2.x, y2 = v2.y, z2 = v2.z;

            int y_len = (int)(y2 - y0) + 1;
            float y = y0;
            for (int _ = 0; _ < y_len; _ ++) {
                float t = (y - y0) / (y2 - y0);
                
                float4 vl = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
                vl.x = t * (x2 - x0) + x0;
                vl.y = y;
                vl.z = t * (z2 - z0) + z0;

                float4 vr = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
                vr.x = t * (x2 - x1) + x1;
                vr.y = y;
                vr.z = t * (z2 - z1) + z1;

                left_point_list[now_line_count] = vl;
                right_point_list[now_line_count] = vr;
                now_line_count ++;
                // printf("ttt : %d\n", now_line_count);
                // printf("tri(%.3f, %.3f, %.3f)  :  vlr (%.3f %.3f) -- (%.3f %.3f)\n", v0.x, v1.x, v2.x, vl.x, vl.y, vr.x, vr.y);
                
                y += 1.0f;
            }
        }
    }
    line_count[idx] = now_line_count;

    // if (now_line_count > screen_height)
    // printf("(%d) : %d\n", idx, now_line_count);
}





__global__ void CudaPhongShaderRS_kernel(
    uint32_t N, uint32_t screen_width, uint32_t screen_height,
    const float4 *left_point, const float4 *right_point, const uint32_t *line_count,
    const float4 *vertex_pos,
    const float3 *vertex_normal,
    const float4 *vertex_color,
    const float2 *vertex_uv,
    lxrr::PhongMaterial **material_p,
    lxrr::Texture **texture_p,
    const float4 *light_vertex_pos,
    const float3 *world_pos,
    const float3 *world_normal,
    const float3 *view_pos,
    float *zbuffer,
    float2 *frag_coord,
    bool *frag_visible,
    float3 *frag_barycentric_coord,
    float4 *frag_color,
    float2 *frag_uv,
    float3 *frag_world_pos,
    float3 *frag_world_normal,
    lxrr::PhongMaterial **frag_material_p,
    lxrr::Texture **frag_texture_p,
    float4 *frag_lp0, float4 *frag_lp1, float4 *frag_lp2,
    int *zbuffer_mutex) {

    auto block = cg::this_thread_block();
    uint32_t idx = block.group_index().x * BLOCK_X + block.thread_index().x;
    if (idx >= (N/3)*screen_height) {
        // printf("---rs if return 000--- (%d, %d)\n", idx, (N/3)*screen_height);
        return;
    }
    uint32_t line_idx = idx % screen_height;
    if(line_idx >= line_count[idx / screen_height]) {
        // printf("---rs if return 111--- (%d, %d)\n", line_idx, line_count[idx / screen_height]);
        return;
    }
    // printf("---rs now idx-line_idx--- (%d, %d)\n", idx, line_idx);

    auto left_p = left_point[idx];
    auto right_p = right_point[idx];
    uint32_t tri_idx = idx / screen_height;
    auto v0_pos = vertex_pos[tri_idx * 3 + 0];
    auto v1_pos = vertex_pos[tri_idx * 3 + 1];
    auto v2_pos = vertex_pos[tri_idx * 3 + 2];
    auto v0_normal = vertex_normal[tri_idx * 3 + 0];
    auto v1_normal = vertex_normal[tri_idx * 3 + 1];
    auto v2_normal = vertex_normal[tri_idx * 3 + 2];
    auto v0_color = vertex_color[tri_idx * 3 + 0];
    auto v1_color = vertex_color[tri_idx * 3 + 1];
    auto v2_color = vertex_color[tri_idx * 3 + 2];
    auto v0_uv = vertex_uv[tri_idx * 3 + 0];
    auto v1_uv = vertex_uv[tri_idx * 3 + 1];
    auto v2_uv = vertex_uv[tri_idx * 3 + 2];
    auto material = material_p[tri_idx * 3];
    auto texture = texture_p[tri_idx * 3];
    auto light_v0_pos = light_vertex_pos[tri_idx * 3 + 0];
    auto light_v1_pos = light_vertex_pos[tri_idx * 3 + 1];
    auto light_v2_pos = light_vertex_pos[tri_idx * 3 + 2];
    auto v0_world_pos = world_pos[tri_idx * 3 + 0];
    auto v1_world_pos = world_pos[tri_idx * 3 + 1];
    auto v2_world_pos = world_pos[tri_idx * 3 + 2];
    auto v0_world_normal = world_normal[tri_idx * 3 + 0];
    auto v1_world_normal = world_normal[tri_idx * 3 + 1];
    auto v2_world_normal = world_normal[tri_idx * 3 + 2];
    auto v0_view_pos = view_pos[tri_idx * 3 + 0];
    auto v1_view_pos = view_pos[tri_idx * 3 + 1];
    auto v2_view_pos = view_pos[tri_idx * 3 + 2];

    ViewpointTransform(v0_pos, screen_width, screen_height);
    ViewpointTransform(v1_pos, screen_width, screen_height);
    ViewpointTransform(v2_pos, screen_width, screen_height);

    float x0 = left_p.x, y0 = left_p.y, z0 = left_p.z;
    float x1 = right_p.x, y1 = right_p.y, z1 = right_p.z;
    
    int dx = x1 - x0;
    int stepx = 1;

    if (dx < 0) {
        stepx = -1;
        dx = -dx;
    }

    int x = x0;
    int y = y0;

    float z_A = v0_view_pos.z;
    float z_B = v1_view_pos.z;
    float z_C = v2_view_pos.z;

    float3 g = CudaCenterOfGravity(Float4ToFloat3(v0_pos), Float4ToFloat3(v1_pos), Float4ToFloat3(v2_pos), make_float2((float)x+0.5f, (float)y+0.5f));
    // printf("RSRSRS::: left(%.3f, %.3f, %.3f) right(%.3f, %.3f, %.3f) g(%.3f %.3f %.3f)\n", x0, y0, z0, x1, y1, z1, g.x, g.y, g.z);
    if (x0 == x1 && !(g.x < 0 || g.x > 1 || g.y < 0 || g.y > 1 || g.z < 0 || g.z > 1)) {
        // printf("-000-before--tri(%.3f,%.3f,%.3f) write pixel (%d, %d)\n", v0_pos.x, v1_pos.x, v2_pos.x, x, y);


        // if (CudaZTestAndWrite(x, y, (CudaMax(left_p.z, right_p.z)+1)/2.0, zbuffer, screen_width, screen_height, zbuffer_mutex)) {
        //     // printf("-000-tri(%.3f,%.3f,%.3f) write pixel (%d, %d)\n", v0_pos.x, v1_pos.x, v2_pos.x, x, y);
        //     float3 g = CudaCenterOfGravity(Float4ToFloat3(v0_pos), Float4ToFloat3(v1_pos), Float4ToFloat3(v2_pos), make_float2((float)x+0.5f, (float)y+0.5f));
        //     int index = y * screen_width + x;
        
        //     frag_visible[index] = true;
        //     AtomicExchFloat3(frag_barycentric_coord[index], g);
        //     AtomicExchFloat4(frag_color[index], Float3ToFloat4(BarycentricLerpFloat3(Float4ToFloat3(v0_color), Float4ToFloat3(v1_color), Float4ToFloat3(v2_color), g, z_A, z_B, z_C)));
        //     AtomicExchFloat2(frag_uv[index], BarycentricLerpFloat2(v0_uv, v1_uv, v2_uv, g, z_A, z_B, z_C));
        //     // if (y == 18) {
        //     //     printf("wirte uv coord(%.1f %.1f) uv(%.3f %.3f)\n", x, y, frag_uv[index].x, frag_uv[index].y);
        //     // }
        //     AtomicExchFloat3(frag_world_pos[index], BarycentricLerpFloat3(v0_world_pos, v1_world_pos, v2_world_pos, g, z_A, z_B, z_C));
        //     AtomicExchFloat3(frag_world_normal[index], BarycentricLerpFloat3(v0_world_normal, v1_world_normal, v2_world_normal, g, z_A, z_B, z_C));

        //     atomicExch((unsigned long long int*)&frag_material_p[index], (unsigned long long int)material);
        //     atomicExch((unsigned long long int*)&frag_texture_p[index], (unsigned long long int)texture);

        //     AtomicExchFloat4(frag_lp0[index], light_v0_pos);
        //     AtomicExchFloat4(frag_lp1[index], light_v1_pos);
        //     AtomicExchFloat4(frag_lp2[index], light_v2_pos);
        // }


        bool blocked = true;
        int _x = CudaClamp<int>(0, screen_width - 1, x), _y = CudaClamp<int>(0, screen_height - 1, y);
        float _z = (CudaMax(left_p.z, right_p.z)+1)/2.0;
        int idx = _y * screen_width + _x;

        while (blocked) {
            if (atomicCAS(&zbuffer_mutex[idx], 0, 1) == 0) {

                if (_z < zbuffer[idx]) {
                    atomicExch(&zbuffer[idx], _z);

                    float3 g = CudaCenterOfGravity(Float4ToFloat3(v0_pos), Float4ToFloat3(v1_pos), Float4ToFloat3(v2_pos), make_float2((float)x+0.5f, (float)y+0.5f));
                
                    frag_visible[idx] = true;
                    AtomicExchFloat3(frag_barycentric_coord[idx], g);
                    AtomicExchFloat4(frag_color[idx], Float3ToFloat4(BarycentricLerpFloat3(Float4ToFloat3(v0_color), Float4ToFloat3(v1_color), Float4ToFloat3(v2_color), g, z_A, z_B, z_C)));
                    AtomicExchFloat2(frag_uv[idx], BarycentricLerpFloat2(v0_uv, v1_uv, v2_uv, g, z_A, z_B, z_C));
                    AtomicExchFloat3(frag_world_pos[idx], BarycentricLerpFloat3(v0_world_pos, v1_world_pos, v2_world_pos, g, z_A, z_B, z_C));
                    AtomicExchFloat3(frag_world_normal[idx], BarycentricLerpFloat3(v0_world_normal, v1_world_normal, v2_world_normal, g, z_A, z_B, z_C));

                    atomicExch((unsigned long long int*)&frag_material_p[idx], (unsigned long long int)material);
                    atomicExch((unsigned long long int*)&frag_texture_p[idx], (unsigned long long int)texture);

                    AtomicExchFloat4(frag_lp0[idx], light_v0_pos);
                    AtomicExchFloat4(frag_lp1[idx], light_v1_pos);
                    AtomicExchFloat4(frag_lp2[idx], light_v2_pos);
                }

                __threadfence();
                atomicExch(&zbuffer_mutex[idx], 0);
                blocked = false;
            }
        }

        return;
    }

    for (int i = 0; i < dx; ++ i, x += stepx) {
        float3 g = CudaCenterOfGravity(Float4ToFloat3(v0_pos), Float4ToFloat3(v1_pos), Float4ToFloat3(v2_pos), make_float2((float)x+0.5f, (float)y+0.5f));
        // if (y == 26) {
            // printf("GPU y26 before test dx(%d) v0(%.3f %.3f %.3f) v1(%.3f %.3f %.3f) v2(%.3f %.3f %.3f) xy(%d %d) g(%.3f %.3f %.3f)\n", dx, v0_pos.x, v0_pos.y, v0_pos.z, v1_pos.x, v1_pos.y, v1_pos.z, v2_pos.x, v2_pos.y, v2_pos.z, x, y, g.x, g.y, g.z); 
        // }
        if (g.x < 0 || g.x > 1 || g.y < 0 || g.y > 1 || g.z < 0 || g.z > 1) {
            continue;
        }
        // if (y == 26) {
        //     printf("GPU y26 g test ok\n");
        // }

        float s = ((float)x - x0) / (x1 - x0);
        // ???????????????????? z or view_z
        float t = s * z0 / (s * z0 + (1 - s) * z1);
        float z = t * (z1 - z0) + z0;
        z = (z + 1) / 2.0f;
        // printf("-111-before--tri(%.3f,%.3f,%.3f) write pixel (%d, %d)\n", v0_pos.x, v1_pos.x, v2_pos.x, x, y);

        // cuda compete
        // if (CudaZTestAndWrite(x, y, z, zbuffer, screen_width, screen_height, zbuffer_mutex)) {
        //     // printf("-111-tri(%.3f,%.3f,%.3f) write pixel (%d, %d)\n", v0_pos.x, v1_pos.x, v2_pos.x, x, y);
        //     int index = y * screen_width + x;
        //     frag_visible[index] = true;
        //     AtomicExchFloat3(frag_barycentric_coord[index], g);
        //     AtomicExchFloat4(frag_color[index], Float3ToFloat4(BarycentricLerpFloat3(Float4ToFloat3(v0_color), Float4ToFloat3(v1_color), Float4ToFloat3(v2_color), g, z_A, z_B, z_C)));
        //     AtomicExchFloat2(frag_uv[index], BarycentricLerpFloat2(v0_uv, v1_uv, v2_uv, g, z_A, z_B, z_C));
        //     // if (y == 18) {
        //         // printf("wirte uv coord(%d %d) uv(%.3f %.3f) g(%.3f %.3f %.3f)\n", x, y, frag_uv[index].x, frag_uv[index].y, g.x, g.y, g.z);
        //     // }
        //     AtomicExchFloat3(frag_world_pos[index], BarycentricLerpFloat3(v0_world_pos, v1_world_pos, v2_world_pos, g, z_A, z_B, z_C));
        //     AtomicExchFloat3(frag_world_normal[index], BarycentricLerpFloat3(v0_world_normal, v1_world_normal, v2_world_normal, g, z_A, z_B, z_C));

        //     atomicExch((unsigned long long int*)&frag_material_p[index], (unsigned long long int)material);
        //     atomicExch((unsigned long long int*)&frag_texture_p[index], (unsigned long long int)texture);

        //     if (y == 48) {
        //         printf("GPU RS xy(%d,48) lp check :: lp0(%.3f %.3f %.3f) lp1(%.3f %.3f %.3f) lp2(%.3f %.3f %.3f) xyz(%d %d %.3f)\n", x, light_v0_pos.x, light_v0_pos.y, light_v0_pos.z, light_v1_pos.x, light_v1_pos.y, light_v1_pos.z, light_v2_pos.x, light_v2_pos.y, light_v2_pos.z, x, y, z);
        //     }

        //     AtomicExchFloat4(frag_lp0[index], light_v0_pos);
        //     AtomicExchFloat4(frag_lp1[index], light_v1_pos);
        //     AtomicExchFloat4(frag_lp2[index], light_v2_pos);
        // }

        bool blocked = true;
        int _x = CudaClamp<int>(0, screen_width - 1, x), _y = CudaClamp<int>(0, screen_height - 1, y);
        float _z = z;
        int idx = _y * screen_width + _x;

        while (blocked) {
            if (atomicCAS(&zbuffer_mutex[idx], 0, 1) == 0) {

                if (_z < zbuffer[idx]) {
                    atomicExch(&zbuffer[idx], _z);

                    frag_visible[idx] = true;
                    AtomicExchFloat3(frag_barycentric_coord[idx], g);
                    AtomicExchFloat4(frag_color[idx], Float3ToFloat4(BarycentricLerpFloat3(Float4ToFloat3(v0_color), Float4ToFloat3(v1_color), Float4ToFloat3(v2_color), g, z_A, z_B, z_C)));
                    AtomicExchFloat2(frag_uv[idx], BarycentricLerpFloat2(v0_uv, v1_uv, v2_uv, g, z_A, z_B, z_C));
                    AtomicExchFloat3(frag_world_pos[idx], BarycentricLerpFloat3(v0_world_pos, v1_world_pos, v2_world_pos, g, z_A, z_B, z_C));
                    AtomicExchFloat3(frag_world_normal[idx], BarycentricLerpFloat3(v0_world_normal, v1_world_normal, v2_world_normal, g, z_A, z_B, z_C));

                    atomicExch((unsigned long long int*)&frag_material_p[idx], (unsigned long long int)material);
                    atomicExch((unsigned long long int*)&frag_texture_p[idx], (unsigned long long int)texture);

                    // if (y == 48) {
                    //     printf("GPU RS xy(%d,48) lp check :: lp0(%.3f %.3f %.3f) lp1(%.3f %.3f %.3f) lp2(%.3f %.3f %.3f) xyz(%d %d %.3f)\n", x, light_v0_pos.x, light_v0_pos.y, light_v0_pos.z, light_v1_pos.x, light_v1_pos.y, light_v1_pos.z, light_v2_pos.x, light_v2_pos.y, light_v2_pos.z, x, y, z);
                    // }

                    AtomicExchFloat4(frag_lp0[idx], light_v0_pos);
                    AtomicExchFloat4(frag_lp1[idx], light_v1_pos);
                    AtomicExchFloat4(frag_lp2[idx], light_v2_pos);
                }

                __threadfence();
                atomicExch(&zbuffer_mutex[idx], 0);
                blocked = false;
            }
        }
    }
}


__global__ void CudaPhongShaderFS_kernel(
    uint32_t screen_width, uint32_t screen_height,
    uint32_t depth_buffer_width, uint32_t depth_buffer_height,
    const float2 *frag_coord,
    const bool *frag_visible,
    const float3 *frag_barycentric_coord,
    const float4 *frag_color,
    const float2 *frag_uv,
    const float3 *frag_world_pos,
    const float3 *frag_world_normal,
    const float4 *frag_lp0,
    const float4 *frag_lp1,
    const float4 *frag_lp2,
    const float *depth_buffer,
    const int *frag_material_id,
    const int *frag_texture_id,
    CudaMaterialStruct *material_list,
    CudaTextureStruct *texture_list,
    size_t *texture_data_start_index,
    float4 *texture_data_ptr,
    const float3 *camera_pos,
    const uint32_t direction_light_count,
    const uint32_t point_light_count,
    const CudaDirectionLightStruct *direction_light_list,
    const CudaPointLightStruct *point_light_list,
    float4 *result_color) {
    // printf("-----------------\n");
    auto block = cg::this_thread_block();
    uint2 thread_idx = {block.group_index().x * BLOCK_X + block.thread_index().x, block.group_index().y * BLOCK_Y + block.thread_index().y};
    // printf("----------(%d %d)\n", thread_idx.x, thread_idx.y);
    // return;
    // printf("FS --- world check: index (%.3f)\n",  frag_world_pos[0].x);
    if (thread_idx.x >= screen_width || thread_idx.y >= screen_height) {
        // printf("----if return <>------(%d %d) (%d %d)\n", thread_idx.x, thread_idx.y, screen_width, screen_height);
        return;
    }
    uint32_t index = thread_idx.y * screen_width + thread_idx.x;

    if (!frag_visible[index]) {
        // if (frag_coord[index].y == 26) printf("----if return visi ------ (%d %d %d)\n", thread_idx.x, thread_idx.y, index);
        return;
    }
    // printf("FS --- no return thread(%d %d) index(%d) coord(%.1f %.1f)\n", thread_idx.x, thread_idx.y, index, frag_coord[index].x, frag_coord[index].y);
    
    auto coord = frag_coord[index];
    auto barycentric_coord = frag_barycentric_coord[index];
    auto color = frag_color[index];
    auto uv = frag_uv[index];
    // printf("FS --- world check: index (%d %.3f)\n", index, frag_lp1[0].x);
    // printf("world pos (%.3f %.3f %.3f) world_normal (%.3f %.3f %.3f)\n", frag_world_pos[index].x, frag_world_pos[index].y, frag_world_pos[index].z, frag_world_normal[index].x, frag_world_normal[index].y, frag_world_normal[index].z);
    // printf("world pos (%.3f %.3f %.3f) world_normal (%.3f %.3f %.3f)\n", frag_world_pos[index].x, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    // return;
    auto world_pos = frag_world_pos[index];
    auto world_normal = frag_world_normal[index];
    auto lp0 = frag_lp0[index];
    auto lp1 = frag_lp1[index];
    auto lp2 = frag_lp2[index];
    auto material = material_list[frag_material_id[index]];
    auto view_pos = *camera_pos;
    auto direction_lights = direction_light_list;
    auto point_lights = point_light_list;
    // printf("GPU lp check --- lp0(%.3f %.3f %.3f) lp1(%.3f %.3f %.3f) lp2(%.3f %.3f %.3f) coord(%.1f %.1f)\n", lp0.x, lp0.y, lp0.z, lp1.x, lp1.y, lp1.z, lp2.x, lp2.y, lp2.z, coord.x, coord.y);
    
    // printf("----------------------\n");
    // if (coord.y == 26)
    //     printf("FS check --- coord(%.1f %.1f)\n", coord.x, coord.y);

    float4 rgb;
    if (frag_texture_id[index] == -1) {
        rgb = make_float4(material.diffuse.x, material.diffuse.y, material.diffuse.z, 1.0f);
    } else {
        auto texture = texture_list[frag_texture_id[index]];
        // printf("texture sample :: w_h(%d %d)\n", texture.width, texture.height);
        rgb = texture.Sample(uv.x, uv.y, texture_data_ptr + texture_data_start_index[frag_texture_id[index]]);
        // printf("texture sample :: coord(%.1f %.1f) uv.xy(%.3f %.3f) start_index(%d) rgb(%.3f %.3f %.3f)\n", coord.x, coord.y, uv.x, uv.y, texture_data_start_index[frag_texture_id[index]], rgb.x, rgb.y, rgb.z);
        // rgb = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    // printf("FS check rgb --- coord(%.1f %.1f) rgb(%.3f %.3f %.3f)\n", coord.x, coord.y, rgb.x, rgb.y, rgb.z);

    float4 render_color = make_float4(0.0f, 0.0f, 0.0f, 1.0f);

    // direction light
    for (uint32_t i = 0; i < direction_light_count; i ++) {
        auto light = direction_lights[i];

        // ambient
        float4 ambient = FloatMultiplyFloat4(light.ka, rgb);

        // diffuse
        float3 normal = Normalized(world_normal);
        float3 l_dir = Normalized(Float3Sub(make_float3(0,0,0), light.dir));
        float4 light_diffuse = FloatMultiplyFloat4(light.kd, light.color);
        float4 flat_diffuse = FloatMultiplyFloat4(fmaxf(CudaDot(normal, l_dir), 0), rgb);
        float4 diffuse = Float4MultiplyFloat4(light_diffuse, flat_diffuse);

        // specular
        float3 v_dir = Normalized(Float3Sub(view_pos, world_pos));
        float3 h = Normalized(Float3Add(v_dir, l_dir));
        float4 light_specular = FloatMultiplyFloat4(light.ks, light.color);
        float4 flat_specular = FloatMultiplyFloat4(powf(fmaxf(CudaDot(normal, h), 0), material.shininess), make_float4(material.specular.x, material.specular.y, material.specular.z, 1.0f));
        float4 specular = Float4MultiplyFloat4(light_specular, flat_specular);

        render_color = Float4Add(render_color, Float4Add(ambient, Float4Add(diffuse, specular)));
        // printf("FS check re_c --- coord(%.1f %.1f) re_c(%.3f %.3f %.3f)\n", coord.x, coord.y, render_color.x, render_color.y, render_color.z);
    }
    // return;
    // point light
    // ......

    // shadow (single direction light)
    if (direction_light_count > 0) {
        auto light = direction_lights[0];

        float3 normal = Normalized(world_normal);
        float3 l_dir = Normalized(Float3Sub(make_float3(0,0,0), light.dir));

        float bias = fmaxf(0.02f * (1.0f - fabsf(CudaDot(normal, l_dir))), 0.005f);

        float pos_on_light_space_x = lp0.x * barycentric_coord.x + lp1.x * barycentric_coord.y + lp2.x * barycentric_coord.z;
        float pos_on_light_space_y = lp0.y * barycentric_coord.x + lp1.y * barycentric_coord.y + lp2.y * barycentric_coord.z;
        float pos_on_light_space_z = lp0.z * barycentric_coord.x + lp1.z * barycentric_coord.y + lp2.z * barycentric_coord.z;
        float4 pos_on_light_space = make_float4(pos_on_light_space_x, pos_on_light_space_y, pos_on_light_space_z, 1.0f);
    
        float shadow = CudaCalculateShadow(pos_on_light_space, bias, depth_buffer, depth_buffer_width, depth_buffer_height);
        // printf("FS check shadow --- coord(%.1f %.1f) lp(%.3f %.3f %.3f) %.3f pos_on_light_space_z_cal(%.3f %.3f %.3f %.3f %.3f %.3f)\n", coord.x, coord.y, pos_on_light_space_x, pos_on_light_space_y, pos_on_light_space_z, shadow, lp0.z, barycentric_coord.x, lp1.z, barycentric_coord.y, lp2.z, barycentric_coord.z);
        render_color = FloatMultiplyFloat4(1 - shadow, render_color);
    }
    // printf("FS check re_c --- coord(%.1f %.1f) re_c(%.3f %.3f %.3f)\n", coord.x, coord.y, render_color.x, render_color.y, render_color.z);

    AtomicExchFloat4(result_color[index], render_color);
}





void CudaPhongShaderVS(uint32_t N, const lxrr::VertIn *in, const lxrr::VertUniform &vert_uniform, lxrr::VertOut *out) {
    float *h_M = new float[16], *h_V = new float[16], *h_P = new float[16];
    float *h_light_V = new float[16], *h_light_P = new float[16];
    Matrix4x4ToFloatPointer(vert_uniform.m, h_M);
    Matrix4x4ToFloatPointer(vert_uniform.v, h_V);
    Matrix4x4ToFloatPointer(vert_uniform.p, h_P);
    Matrix4x4ToFloatPointer(vert_uniform.light_V, h_light_V);
    Matrix4x4ToFloatPointer(vert_uniform.light_P, h_light_P);

    float4 *h_vertex_pos = new float4[N];
    float3 *h_vertex_normal = new float3[N];
    float4 *h_light_vertex_pos = new float4[N];

    float4 *h_result_vertex_pos = new float4[N];
    float4 *h_result_light_vertex_pos = new float4[N];
    float3 *h_result_world_pos = new float3[N];
    float3 *h_result_world_normal = new float3[N];
    float3 *h_result_view_pos = new float3[N];

    for (size_t i = 0; i < N; i ++) {
        Vector4ToFloatPointer(in[i].vertex->pos, h_vertex_pos[i]);
        Vector3ToFloatPointer(in[i].vertex->normal, h_vertex_normal[i]);
        Vector4ToFloatPointer(*in[i].light_vertex_pos, h_light_vertex_pos[i]);
    }

    // std::cout << "CudaPhongShaderVS" << std::endl;

    float *d_M, *d_V, *d_P;
    float *d_light_V, *d_light_P;
    float4 *d_vertex_pos;
    float3 *d_vertex_normal;
    float4 *d_light_vertex_pos;
    float4 *d_result_vertex_pos;
    float4 *d_result_light_vertex_pos;
    float3 *d_result_world_pos;
    float3 *d_result_world_normal;
    float3 *d_result_view_pos;
    CHECK(cudaMalloc(&d_M, 16 * sizeof(float)));
    CHECK(cudaMalloc(&d_V, 16 * sizeof(float)));
    CHECK(cudaMalloc(&d_P, 16 * sizeof(float)));
    CHECK(cudaMalloc(&d_light_V, 16 * sizeof(float)));
    CHECK(cudaMalloc(&d_light_P, 16 * sizeof(float)));
    CHECK(cudaMalloc(&d_vertex_pos, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_vertex_normal, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_light_vertex_pos, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_result_vertex_pos, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_result_light_vertex_pos, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_result_world_pos, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_result_world_normal, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_result_view_pos, N * sizeof(float3)));


    CHECK(cudaMemcpy(d_M, h_M, 16 * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_V, h_V, 16 * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_P, h_P, 16 * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_light_V, h_light_V, 16 * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_light_P, h_light_P, 16 * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_vertex_pos, h_vertex_pos, N * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_vertex_normal, h_vertex_normal, N * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_light_vertex_pos, h_light_vertex_pos, N * sizeof(float4), cudaMemcpyHostToDevice));


    dim3 block(BLOCK_X);
    dim3 grid((N-1)/BLOCK_X+1);
    CudaPhongShaderVS_kernel<<<grid, block>>>(
        N,
        d_M, d_V, d_P,
        d_light_V, d_light_P,
        d_vertex_pos, d_vertex_normal,
        d_light_vertex_pos,
        d_result_vertex_pos,
        d_result_light_vertex_pos,
        d_result_world_pos,
        d_result_world_normal,
        d_result_view_pos
    );
    CHECK(cudaDeviceSynchronize());

    CHECK(cudaMemcpy(h_result_vertex_pos, d_result_vertex_pos, N * sizeof(float4), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_result_light_vertex_pos, d_result_light_vertex_pos, N * sizeof(float4), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_result_world_pos, d_result_world_pos, N * sizeof(float3), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_result_world_normal, d_result_world_normal, N * sizeof(float3), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_result_view_pos, d_result_view_pos, N * sizeof(float3), cudaMemcpyDeviceToHost));

    for (int i = 0; i < N; i ++) {
        // printf("CUDAVS::: %d: %f %f %f\n", i, h_result_vertex_pos[i].x, h_result_vertex_pos[i].y, h_result_vertex_pos[i].z);
        FloatPointerToVector4(h_result_vertex_pos[i], out[i].vertex->pos);
        FloatPointerToVector4(h_result_light_vertex_pos[i], *out[i].light_vertex_pos);
        FloatPointerToVector3(h_result_world_pos[i], *out[i].world_pos);
        FloatPointerToVector3(h_result_world_normal[i], *out[i].world_normal);
        FloatPointerToVector3(h_result_view_pos[i], *out[i].view_pos);
        // printf("CUDAVS back::: %d: %f %f %f\n", i, out[i].vertex->pos.x(), out[i].vertex->pos.y(), out[i].vertex->pos.z());
    }

    delete[] h_M;
    delete[] h_V;
    delete[] h_P;
    delete[] h_light_V;
    delete[] h_light_P;
    delete[] h_vertex_pos;
    delete[] h_vertex_normal;
    delete[] h_light_vertex_pos;
    delete[] h_result_vertex_pos;
    delete[] h_result_light_vertex_pos;
    delete[] h_result_world_pos;
    delete[] h_result_world_normal;
    delete[] h_result_view_pos;
    cudaFree(d_M);
    cudaFree(d_V);
    cudaFree(d_P);
    cudaFree(d_light_V);
    cudaFree(d_light_P);
    cudaFree(d_vertex_pos);
    cudaFree(d_vertex_normal);
    cudaFree(d_light_vertex_pos);
    cudaFree(d_result_vertex_pos);
    cudaFree(d_result_light_vertex_pos);
    cudaFree(d_result_world_pos);
    cudaFree(d_result_world_normal);
    cudaFree(d_result_view_pos);
}


void CudaPhongShaderRS(const lxrr::RasterizationIn &in, const lxrr::RasterizationUniform &rasterization_uniform, lxrr::RasterizationOut *out) {
    // Uniform
    uint32_t screen_width = rasterization_uniform.screen_width;
    uint32_t screen_height = rasterization_uniform.screen_height;
    auto zbuffer = rasterization_uniform.zbuffer;
    float *h_zbuffer = new float[zbuffer->GetWidth() * zbuffer->GetHeight()];
    for (int i = 0; i < zbuffer->GetWidth() * zbuffer->GetHeight(); i ++) {
        h_zbuffer[i] = zbuffer->Get(i / zbuffer->GetWidth(), i % zbuffer->GetWidth());
    }

    // input
    uint32_t N = in.vertex_list->size();
    float4 *h_vertex_pos = new float4[N];
    float3 *h_vertex_normal = new float3[N];
    float4 *h_vertex_color = new float4[N];
    float2 *h_vertex_uv = new float2[N];
    lxrr::PhongMaterial **h_material_p = new lxrr::PhongMaterial*[N];
    lxrr::Texture **h_texture_p = new lxrr::Texture*[N];

    float4 *h_light_vertex_pos = new float4[N];
    float3 *h_world_pos = new float3[N];
    float3 *h_world_normal = new float3[N];
    float3 *h_view_pos = new float3[N];

    for (size_t i = 0; i < N; i ++) {
        Vector4ToFloatPointer(in.vertex_list->at(i)->pos, h_vertex_pos[i]);
        Vector3ToFloatPointer(in.vertex_list->at(i)->normal, h_vertex_normal[i]);
        Vector4ToFloatPointer(in.vertex_list->at(i)->color, h_vertex_color[i]);
        h_vertex_uv[i] = make_float2(in.vertex_list->at(i)->uv.x(), in.vertex_list->at(i)->uv.y());
        h_material_p[i] = in.vertex_list->at(i)->material.get();
        h_texture_p[i] = in.vertex_list->at(i)->texture.get();

        Vector4ToFloatPointer(*in.light_vertex_pos_list->at(i), h_light_vertex_pos[i]);
        Vector3ToFloatPointer(*in.world_pos_list->at(i), h_world_pos[i]);
        Vector3ToFloatPointer(*in.world_normal_list->at(i), h_world_normal[i]);
        Vector3ToFloatPointer(*in.view_pos_list->at(i), h_view_pos[i]);
    }

    uint32_t M = screen_width * screen_height;
    float2 *h_frag_coord = new float2[M];
    bool *h_frag_visible = new bool[M];
    float3 *h_frag_barycentric_coord_list = new float3[M];
    float4 *h_frag_color = new float4[M];
    float2 *h_frag_uv = new float2[M];
    float3 *h_frag_world_pos = new float3[M];
    float3 *h_frag_world_normal = new float3[M];
    lxrr::PhongMaterial **h_frag_material_p = new lxrr::PhongMaterial*[M];
    lxrr::Texture **h_frag_texture_p = new lxrr::Texture*[M];
    float4 *h_frag_lp0 = new float4[M];
    float4 *h_frag_lp1 = new float4[M];
    float4 *h_frag_lp2 = new float4[M];

    for (size_t i = 0; i < M; i ++) {
        h_frag_coord[i] = make_float2(i % screen_width, i / screen_width);
        h_frag_visible[i] = false;
        h_frag_barycentric_coord_list[i] = make_float3(1, 0, 0);
        h_frag_color[i] = make_float4(0, 0, 0, 1);
        h_frag_uv[i] = make_float2(0, 0);
        h_frag_world_pos[i] = make_float3(0, 0, 0);
        h_frag_world_normal[i] = make_float3(0, 0, 0);
        h_frag_material_p[i] = nullptr;
        h_frag_texture_p[i] = nullptr;
        h_frag_lp0[i] = make_float4(0, 0, 0, 1);
        h_frag_lp1[i] = make_float4(0, 0, 0, 1);
        h_frag_lp2[i] = make_float4(0, 0, 0, 1);
    }


    float4 *d_vertex_pos;
    float3 *d_vertex_normal;
    float4 *d_vertex_color;
    float2 *d_vertex_uv;
    lxrr::PhongMaterial **d_material_p;
    lxrr::Texture **d_texture_p;
    float4 *d_light_vertex_pos;
    float3 *d_world_pos, *d_world_normal, *d_view_pos;
    float *d_zbuffer;
    float2 *d_frag_coord;
    bool *d_frag_visible;
    float3 *d_frag_barycentric_coord_list;
    float4 *d_frag_color;
    float2 *d_frag_uv;
    float3 *d_frag_world_pos, *d_frag_world_normal;
    lxrr::PhongMaterial **d_frag_material_p;
    lxrr::Texture **d_frag_texture_p;
    float4 *d_frag_lp0, *d_frag_lp1, *d_frag_lp2;
    int *d_zbuffer_mutex;

    CHECK(cudaMalloc(&d_vertex_pos, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_vertex_normal, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_vertex_color, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_vertex_uv, N * sizeof(float2)));
    CHECK(cudaMalloc(&d_material_p, N * sizeof(lxrr::PhongMaterial*)));
    CHECK(cudaMalloc(&d_texture_p, N * sizeof(lxrr::Texture*)));
    CHECK(cudaMalloc(&d_light_vertex_pos, N * sizeof(float4)));
    CHECK(cudaMalloc(&d_world_pos, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_world_normal, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_view_pos, N * sizeof(float3)));
    CHECK(cudaMalloc(&d_zbuffer, screen_width * screen_height * sizeof(float)));
    CHECK(cudaMalloc(&d_frag_coord, M * sizeof(float2)));
    CHECK(cudaMalloc(&d_frag_visible, M * sizeof(bool)));
    CHECK(cudaMalloc(&d_frag_barycentric_coord_list, M * sizeof(float3)));
    CHECK(cudaMalloc(&d_frag_color, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_uv, M * sizeof(float2)));
    CHECK(cudaMalloc(&d_frag_world_pos, M * sizeof(float3)));
    CHECK(cudaMalloc(&d_frag_world_normal, M * sizeof(float3)));
    CHECK(cudaMalloc(&d_frag_material_p, M * sizeof(lxrr::PhongMaterial*)));
    CHECK(cudaMalloc(&d_frag_texture_p, M * sizeof(lxrr::Texture*)));
    CHECK(cudaMalloc(&d_frag_lp0, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_lp1, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_lp2, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_zbuffer_mutex, M * sizeof(int)));

    CHECK(cudaMemcpy(d_vertex_pos, h_vertex_pos, N * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_vertex_normal, h_vertex_normal, N * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_vertex_color, h_vertex_color, N * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_vertex_uv, h_vertex_uv, N * sizeof(float2), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_material_p, h_material_p, N * sizeof(lxrr::PhongMaterial*), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_texture_p, h_texture_p, N * sizeof(lxrr::Texture*), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_light_vertex_pos, h_light_vertex_pos, N * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_world_pos, h_world_pos, N * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_world_normal, h_world_normal, N * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_view_pos, h_view_pos, N * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_zbuffer, h_zbuffer, screen_width * screen_height * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_coord, h_frag_coord, M * sizeof(float2), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_visible, h_frag_visible, M * sizeof(bool), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_barycentric_coord_list, h_frag_barycentric_coord_list, M * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_color, h_frag_color, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_uv, h_frag_uv, M * sizeof(float2), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_world_pos, h_frag_world_pos, M * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_world_normal, h_frag_world_normal, M * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_material_p, h_frag_material_p, M * sizeof(lxrr::PhongMaterial*), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_texture_p, h_frag_texture_p, M * sizeof(lxrr::Texture*), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_lp0, h_frag_lp0, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_lp1, h_frag_lp1, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_lp2, h_frag_lp2, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemset(d_zbuffer_mutex, 0, M * sizeof(int)));

    float4 *d_left_point;
    float4 *d_right_point;
    uint32_t *d_line_count;
    CHECK(cudaMalloc(&d_left_point, (N / 3) * screen_height * sizeof(float4)));
    CHECK(cudaMalloc(&d_right_point, (N / 3) * screen_height * sizeof(float4)));
    CHECK(cudaMalloc(&d_line_count, (N / 3) * sizeof(uint32_t)));

    dim3 block(BLOCK_X);
    dim3 grid(((N/3)-1)/BLOCK_X+1);
    CudaPhongShaderRS_pre_kernel<<<grid, block>>>(
        N, screen_width, screen_height,
        d_vertex_pos,
        d_left_point,
        d_right_point,
        d_line_count
    );
    CHECK(cudaDeviceSynchronize());

    dim3 block2(BLOCK_X);
    dim3 grid2(((N/3)*screen_height-1)/BLOCK_X+1);
    CudaPhongShaderRS_kernel<<<grid2, block2>>>(
        N, screen_width, screen_height,
        d_left_point, d_right_point, d_line_count,
        d_vertex_pos,
        d_vertex_normal,
        d_vertex_color,
        d_vertex_uv,
        d_material_p,
        d_texture_p,
        d_light_vertex_pos,
        d_world_pos,
        d_world_normal,
        d_view_pos,
        d_zbuffer,
        d_frag_coord,
        d_frag_visible,
        d_frag_barycentric_coord_list,
        d_frag_color,
        d_frag_uv,
        d_frag_world_pos,
        d_frag_world_normal,
        d_frag_material_p,
        d_frag_texture_p,
        d_frag_lp0, d_frag_lp1, d_frag_lp2,
        d_zbuffer_mutex
    );
    CHECK(cudaDeviceSynchronize());

    CHECK(cudaMemcpy(h_frag_coord, d_frag_coord, M * sizeof(float2), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_visible, d_frag_visible, M * sizeof(bool), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_barycentric_coord_list, d_frag_barycentric_coord_list, M * sizeof(float3), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_color, d_frag_color, M * sizeof(float4), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_uv, d_frag_uv, M * sizeof(float2), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_world_pos, d_frag_world_pos, M * sizeof(float3), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_world_normal, d_frag_world_normal, M * sizeof(float3), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_material_p, d_frag_material_p, M * sizeof(lxrr::PhongMaterial*), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_texture_p, d_frag_texture_p, M * sizeof(lxrr::Texture*), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_lp0, d_frag_lp0, M * sizeof(float4), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_lp1, d_frag_lp1, M * sizeof(float4), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_frag_lp2, d_frag_lp2, M * sizeof(float4), cudaMemcpyDeviceToHost));
    CHECK(cudaMemcpy(h_zbuffer, d_zbuffer, screen_width * screen_height * sizeof(float), cudaMemcpyDeviceToHost));

    for (size_t i = 0; i < M; i ++) {
        if (h_frag_visible[i]) {
            // printf("---frag visible--- i = %d\n", i);
            *out->frag_coord_list->at(i) = lxrr::vec2(h_frag_coord[i].x, h_frag_coord[i].y);
            out->frag_visible_list->at(i) = h_frag_visible[i];
            FloatPointerToVector3(h_frag_barycentric_coord_list[i], *out->frag_barycentric_coord_list->at(i));
            FloatPointerToVector4(h_frag_color[i], *out->frag_color_list->at(i));
            *out->frag_uv_list->at(i) = lxrr::vec2(h_frag_uv[i].x, h_frag_uv[i].y);
            FloatPointerToVector3(h_frag_world_pos[i], *out->frag_world_pos_list->at(i));
            FloatPointerToVector3(h_frag_world_normal[i], *out->frag_world_normal_list->at(i));
            out->frag_material_list->at(i) = std::make_shared<lxrr::PhongMaterial>(*h_frag_material_p[i]);
            out->frag_texture_list->at(i) = std::make_shared<lxrr::Texture>(*h_frag_texture_p[i]);
            FloatPointerToVector4(h_frag_lp0[i], *out->lp0_list->at(i));
            FloatPointerToVector4(h_frag_lp1[i], *out->lp1_list->at(i));
            FloatPointerToVector4(h_frag_lp2[i], *out->lp2_list->at(i));
        }
    }
    for (size_t i = 0; i < screen_width * screen_height; i ++) {
        rasterization_uniform.zbuffer->Write(i / screen_width, i % screen_width, h_zbuffer[i]);
    }

    delete[] h_zbuffer;
    delete[] h_vertex_pos;
    delete[] h_vertex_normal;
    delete[] h_vertex_color;
    delete[] h_vertex_uv;
    delete[] h_material_p;
    delete[] h_texture_p;
    delete[] h_light_vertex_pos;
    delete[] h_world_pos;
    delete[] h_world_normal;
    delete[] h_view_pos;
    delete[] h_frag_coord;
    delete[] h_frag_visible;
    delete[] h_frag_barycentric_coord_list;
    delete[] h_frag_color;
    delete[] h_frag_uv;
    delete[] h_frag_world_pos;
    delete[] h_frag_world_normal;
    delete[] h_frag_material_p;
    delete[] h_frag_texture_p;
    delete[] h_frag_lp0;
    delete[] h_frag_lp1;
    delete[] h_frag_lp2;
    cudaFree(d_vertex_pos);
    cudaFree(d_vertex_normal);
    cudaFree(d_vertex_color);
    cudaFree(d_vertex_uv);
    cudaFree(d_material_p);
    cudaFree(d_texture_p);
    cudaFree(d_light_vertex_pos);
    cudaFree(d_world_pos);
    cudaFree(d_world_normal);
    cudaFree(d_view_pos);
    cudaFree(d_zbuffer);
    cudaFree(d_frag_coord);
    cudaFree(d_frag_visible);
    cudaFree(d_frag_barycentric_coord_list);
    cudaFree(d_frag_color);
    cudaFree(d_frag_uv);
    cudaFree(d_frag_world_pos);
    cudaFree(d_frag_world_normal);
    cudaFree(d_frag_material_p);
    cudaFree(d_frag_texture_p);
    cudaFree(d_frag_lp0);
    cudaFree(d_frag_lp1);
    cudaFree(d_frag_lp2);
    cudaFree(d_left_point);
    cudaFree(d_right_point);
    cudaFree(d_line_count);
    cudaFree(d_zbuffer_mutex);
}


void CudaPhongShaderFS(uint32_t screen_width, uint32_t screen_height, std::vector<bool> &frag_visible_list, const lxrr::FragIn *in, const lxrr::FragUniform &frag_uniform, lxrr::VertUniform &vert_uniform, lxrr::FragOut *out) {
    // printf("---CudaPhongShaderFS start -1-1 --\n");
    uint32_t M = screen_width * screen_height;
    uint32_t depth_buffer_width = frag_uniform.depth_buffer->GetWidth(), depth_buffer_height = frag_uniform.depth_buffer->GetHeight();;
    float2 *h_frag_coord = new float2[M];
    bool *h_frag_visible = new bool[M];
    float3 *h_frag_barycentric_coord = new float3[M];
    float4 *h_frag_color = new float4[M];
    float2 *h_frag_uv = new float2[M];
    float3 *h_frag_world_pos = new float3[M];
    float3 *h_frag_world_normal = new float3[M];
    float4 *h_frag_lp0 = new float4[M];
    float4 *h_frag_lp1 = new float4[M];
    float4 *h_frag_lp2 = new float4[M];
    float *h_depth_buffer = new float[depth_buffer_width * depth_buffer_height];
    float4 *h_result_color = new float4[M];
    float3 h_camera_pos;
    CudaDirectionLightStruct *h_direction_light_list = new CudaDirectionLightStruct[vert_uniform.direction_light_list->size()];
    CudaPointLightStruct *h_point_light_list = new CudaPointLightStruct[vert_uniform.point_light_list->size()];

    // printf("---CudaPhongShaderFS start --\n");
    for (size_t i = 0; i < M; i ++) {
        h_frag_coord[i] = make_float2(in[i].frag_coord->x(), in[i].frag_coord->y());
        // printf("h_frag_coord test :: i %d :: %.3f %.3f\n", i, in[i].frag_coord->x(), in[i].frag_coord->y());
        h_frag_visible[i] = frag_visible_list[i];
        Vector3ToFloatPointer(*in[i].frag_barycentric_coord, h_frag_barycentric_coord[i]);
        Vector4ToFloatPointer(*in[i].frag_color, h_frag_color[i]);
        h_frag_uv[i] = make_float2(in[i].frag_uv->x(), in[i].frag_uv->y());
        Vector3ToFloatPointer(*in[i].frag_world_pos, h_frag_world_pos[i]);
        Vector3ToFloatPointer(*in[i].frag_world_normal, h_frag_world_normal[i]);
        Vector4ToFloatPointer(*in[i].lp0, h_frag_lp0[i]);
        Vector4ToFloatPointer(*in[i].lp1, h_frag_lp1[i]);
        Vector4ToFloatPointer(*in[i].lp2, h_frag_lp2[i]);
        h_result_color[i] = make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    for (size_t i = 0; i < depth_buffer_width * depth_buffer_height; i ++) {
        h_depth_buffer[i] = frag_uniform.depth_buffer->Get(i / depth_buffer_width, i % depth_buffer_width);
    }

    // printf("CPU: world_pos : %.3f\n", h_frag_world_pos[600].x);

    uint32_t direction_light_count = vert_uniform.direction_light_list->size();
    uint32_t point_light_count = vert_uniform.point_light_list->size();
    Vector3ToFloatPointer(vert_uniform.camera_pos, h_camera_pos);
    for (size_t i = 0; i < direction_light_count; i ++) {
        Vector3ToFloatPointer(vert_uniform.direction_light_list->at(i)->get_pos(), h_direction_light_list[i].pos);
        Vector3ToFloatPointer(vert_uniform.direction_light_list->at(i)->get_dir(), h_direction_light_list[i].dir);
        h_direction_light_list[i].intensity = vert_uniform.direction_light_list->at(i)->get_intensity();
        Vector4ToFloatPointer(vert_uniform.direction_light_list->at(i)->get_color(), h_direction_light_list[i].color);
        h_direction_light_list[i].ka = vert_uniform.direction_light_list->at(i)->get_ka();
        h_direction_light_list[i].kd = vert_uniform.direction_light_list->at(i)->get_kd();
        h_direction_light_list[i].ks = vert_uniform.direction_light_list->at(i)->get_ks();
    }
    for (size_t i = 0; i < point_light_count; i ++) {
        Vector3ToFloatPointer(vert_uniform.point_light_list->at(i)->get_pos(), h_point_light_list[i].pos);
        h_point_light_list[i].intensity = vert_uniform.point_light_list->at(i)->get_intensity();
        Vector4ToFloatPointer(vert_uniform.point_light_list->at(i)->get_color(), h_point_light_list[i].color);
        h_point_light_list[i].ka = vert_uniform.point_light_list->at(i)->get_ka();
        h_point_light_list[i].kd = vert_uniform.point_light_list->at(i)->get_kd();
        h_point_light_list[i].ks = vert_uniform.point_light_list->at(i)->get_ks();
    }
    // printf("---CudaPhongShaderFS start 1111 --\n");
    int *h_frag_material_id = new int[M];
    int *h_frag_texture_id = new int[M];
    std::vector<CudaMaterialStruct> material_list;
    std::vector<CudaTextureStruct> texture_list;
    std::vector<size_t> frag_in_id_for_texture_data;
    std::unordered_map<unsigned int, int> material_id_map;
    std::unordered_map<unsigned int, int> texture_id_map;
    for (size_t i = 0; i < M; i ++) {
        auto m_id = in[i].frag_material->id;
        if (!material_id_map.contains(m_id)) {
            float3 ambient, diffuse, specular;
            Vector3ToFloatPointer(in[i].frag_material->ambient, ambient);
            Vector3ToFloatPointer(in[i].frag_material->diffuse, diffuse);
            Vector3ToFloatPointer(in[i].frag_material->specular, specular);
            float shininess = in[i].frag_material->shininess;
            material_list.push_back({ambient, diffuse, specular, shininess});
            material_id_map[m_id] = material_list.size() - 1;
        }
        h_frag_material_id[i] = material_id_map[m_id];

        if (in[i].frag_texture) { 
            auto t_id = in[i].frag_texture->id;
            if (!texture_id_map.contains(t_id)) {
                auto width = in[i].frag_texture->GetWidth(), height = in[i].frag_texture->GetHeight();
                texture_list.push_back({width, height});
                // for (size_t j = 0; j < width * height; j ++) {
                //     // printf("--- texture sample --- %d %.3f %.3f\n", i, in[i].frag_texture->Sample(j / width, j % width), texture_list.back()->data[j]);
                //     Vector4ToFloatPointer(in[i].frag_texture->Sample(j / width, j % width), texture_list.back()->data[j]);
                // }
                texture_id_map[t_id] = texture_list.size() - 1;
                frag_in_id_for_texture_data.push_back(i);
            }
            h_frag_texture_id[i] = texture_id_map[t_id];
        } else {
            h_frag_texture_id[i] = -1;
        }
    }
    size_t *texture_data_start_index = new size_t[texture_list.size()];
    size_t texture_data_size = 0;
    for (size_t i = 0; i < texture_list.size(); i ++) {
        texture_data_start_index[i] = texture_data_size;
        texture_data_size += texture_list[i].width * texture_list[i].height;
    }
    float4 *texture_data_ptr = new float4[texture_data_size];
    // printf("HOST :: texture_data_size : %d\n", texture_data_size);
    for (size_t i = 0; i < texture_list.size(); i ++) {
        auto now_texture_data = texture_data_ptr + texture_data_start_index[i];
        for (size_t j = 0; j < texture_list[i].width * texture_list[i].height; j ++) {
            Vector4ToFloatPointer(in[frag_in_id_for_texture_data[i]].frag_texture->GetTextureDataPtr()[j], now_texture_data[j]);
        }
        // printf("HOST :: now_texture_data pos(%d, %d)")
    }

    // printf("---CudaPhongShaderFS start 2222 --\n");
    float2 *d_frag_coord;
    bool *d_frag_visible;
    float3 *d_frag_barycentric_coord;
    float4 *d_frag_color;
    float2 *d_frag_uv;
    float3 *d_frag_world_pos;
    float3 *d_frag_world_normal;
    float4 *d_frag_lp0;
    float4 *d_frag_lp1;
    float4 *d_frag_lp2;
    float *d_depth_buffer;
    float4 *d_result_color;
    int *d_frag_material_id;
    int *d_frag_texture_id;
    CudaMaterialStruct *d_material_list;
    CudaTextureStruct *d_texture_list;
    size_t *d_texture_data_start_index;
    float4 *d_texture_data_ptr;

    float3 *d_camera_pos;
    CudaDirectionLightStruct *d_direction_light_list;
    CudaPointLightStruct *d_point_light_list;

    CHECK(cudaMalloc(&d_frag_coord, M * sizeof(float2)));
    CHECK(cudaMalloc(&d_frag_visible, M * sizeof(bool)));
    CHECK(cudaMalloc(&d_frag_barycentric_coord, M * sizeof(float3)));
    CHECK(cudaMalloc(&d_frag_color, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_uv, M * sizeof(float2)));
    CHECK(cudaMalloc(&d_frag_world_pos, M * sizeof(float3)));
    CHECK(cudaMalloc(&d_frag_world_normal, M * sizeof(float3)));
    CHECK(cudaMalloc(&d_frag_lp0, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_lp1, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_lp2, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_depth_buffer, depth_buffer_width * depth_buffer_height * sizeof(float)));
    CHECK(cudaMalloc(&d_result_color, M * sizeof(float4)));
    CHECK(cudaMalloc(&d_frag_material_id, M * sizeof(int)));
    CHECK(cudaMalloc(&d_frag_texture_id, M * sizeof(int)));
    CHECK(cudaMalloc(&d_material_list, material_list.size() * sizeof(CudaMaterialStruct)));
    CHECK(cudaMalloc(&d_texture_list, texture_list.size() * sizeof(CudaTextureStruct)));
    CHECK(cudaMalloc(&d_texture_data_start_index, texture_list.size() * sizeof(size_t)));
    CHECK(cudaMalloc(&d_texture_data_ptr, texture_data_size * sizeof(float4)));
    CHECK(cudaMalloc(&d_camera_pos, sizeof(float3)));
    CHECK(cudaMalloc(&d_direction_light_list, direction_light_count * sizeof(CudaDirectionLightStruct)));
    CHECK(cudaMalloc(&d_point_light_list, point_light_count * sizeof(CudaPointLightStruct)));

    CHECK(cudaMemcpy(d_frag_coord, h_frag_coord, M * sizeof(float2), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_visible, h_frag_visible, M * sizeof(bool), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_barycentric_coord, h_frag_barycentric_coord, M * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_color, h_frag_color, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_uv, h_frag_uv, M * sizeof(float2), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_world_pos, h_frag_world_pos, M * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_world_normal, h_frag_world_normal, M * sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_lp0, h_frag_lp0, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_lp1, h_frag_lp1, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_lp2, h_frag_lp2, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_depth_buffer, h_depth_buffer, frag_uniform.depth_buffer->GetWidth() * frag_uniform.depth_buffer->GetHeight() * sizeof(float), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_result_color, h_result_color, M * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_material_id, h_frag_material_id, M * sizeof(int), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_frag_texture_id, h_frag_texture_id, M * sizeof(int), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_material_list, material_list.data(), material_list.size() * sizeof(CudaMaterialStruct), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_texture_list, texture_list.data(), texture_list.size() * sizeof(CudaTextureStruct), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_texture_data_start_index, texture_data_start_index, texture_list.size() * sizeof(size_t), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_texture_data_ptr, texture_data_ptr, texture_data_size * sizeof(float4), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_camera_pos, &h_camera_pos, sizeof(float3), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_direction_light_list, h_direction_light_list, direction_light_count * sizeof(CudaDirectionLightStruct), cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_point_light_list, h_point_light_list, point_light_count * sizeof(CudaPointLightStruct), cudaMemcpyHostToDevice));
    
    dim3 block(BLOCK_X, BLOCK_Y);
    dim3 grid((screen_width-1)/BLOCK_X+1, (screen_height-1)/BLOCK_Y+1);
    // printf("FS block grid --- %d %d %d %d\n", block.x, block.y, grid.x, grid.y);
    CudaPhongShaderFS_kernel<<<grid, block>>>(
        screen_width, screen_height,
        depth_buffer_width, depth_buffer_height,
        d_frag_coord,
        d_frag_visible,
        d_frag_barycentric_coord,
        d_frag_color,
        d_frag_uv,
        d_frag_world_pos,
        d_frag_world_normal,
        d_frag_lp0,
        d_frag_lp1,
        d_frag_lp2,
        d_depth_buffer,
        d_frag_material_id,
        d_frag_texture_id,
        d_material_list,
        d_texture_list,
        d_texture_data_start_index,
        d_texture_data_ptr,
        d_camera_pos,
        direction_light_count,
        point_light_count,
        d_direction_light_list,
        d_point_light_list,
        d_result_color
    );
    CHECK(cudaDeviceSynchronize());

    CHECK(cudaMemcpy(h_result_color, d_result_color, M * sizeof(float4), cudaMemcpyDeviceToHost));
    for (size_t i = 0; i < M; i ++) {
        FloatPointerToVector4(h_result_color[i], *out[i].color);
    }

    delete[] h_frag_coord;
    delete[] h_frag_visible;
    delete[] h_frag_barycentric_coord;
    delete[] h_frag_color;
    delete[] h_frag_uv;
    delete[] h_frag_world_pos;
    delete[] h_frag_world_normal;
    delete[] h_frag_lp0;
    delete[] h_frag_lp1;
    delete[] h_frag_lp2;
    delete[] h_depth_buffer;
    delete[] h_result_color;
    delete[] h_direction_light_list;
    delete[] h_point_light_list;
    delete[] h_frag_material_id;
    delete[] h_frag_texture_id;
    material_list.clear();
    texture_list.clear();
    delete[] texture_data_start_index;
    delete[] texture_data_ptr;
    cudaFree(d_frag_coord);
    cudaFree(d_frag_visible);
    cudaFree(d_frag_barycentric_coord);
    cudaFree(d_frag_color);
    cudaFree(d_frag_uv);
    cudaFree(d_frag_world_pos);
    cudaFree(d_frag_world_normal);
    cudaFree(d_frag_lp0);
    cudaFree(d_frag_lp1);
    cudaFree(d_frag_lp2);
    cudaFree(d_depth_buffer);
    cudaFree(d_result_color);
    cudaFree(d_frag_material_id);
    cudaFree(d_frag_texture_id);
    cudaFree(d_material_list);
    cudaFree(d_texture_list);
    cudaFree(d_camera_pos);
    cudaFree(d_direction_light_list);
    cudaFree(d_point_light_list);
}

} // namespace lxrr_cuda