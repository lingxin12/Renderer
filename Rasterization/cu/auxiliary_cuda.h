#include <cstdio>
#include "cuda.h"
#include "cuda_runtime.h"
#include "vector_types.h"
#include "material.h"

namespace lxrr_cuda {

#define BLOCK_X 16
#define BLOCK_Y 16


// struct DepthBufferLock {

// }


struct CudaTextureStruct {
    uint32_t width, height;

    __device__ inline float4 Sample(float u, float v, const float4* data) const {
        if (u < 0) u = 0;
        if (u > 0.999999f) u = 0.999999f;
        if (v < 0) v = 0;
        if (v > 0.999999f) v = 0.999999f;

        int i = (int)(u * width);
        int j = (int)(v * height);

        // printf("Cuda Sample :: u %.3f v %.3f i %d, j %d value(%.3f %.3f %.3f %.3f)\n", u, v, i, j, data[j * width + i].x, data[j * width + i].y, data[j * width + i].z, data[j * width + i].w);
        
        return data[j * width + i];
    }
};

struct CudaMaterialStruct {
    float3 ambient, diffuse, specular;
    float shininess;
};

struct CudaDirectionLightStruct {
    float3 pos;
    float3 dir;
    float intensity;
    float4 color;
    float ka, kd, ks;
};

struct CudaPointLightStruct {
    float3 pos;
    float intensity;
    float4 color;
    float ka, kd, ks;
};

__forceinline__ __device__ float4 StandardizationVector(const float4 &v) {
    return make_float4(v.x / v.w, v.y / v.w, v.z / v.w, 1.0f);
}

// M is 4x4
__forceinline__ __device__ float3 TransformVector3x3(const float* m, const float3 &v) {
    return make_float3(m[0]*v.x + m[1]*v.y + m[2]*v.z,
                       m[4]*v.x + m[5]*v.y + m[6]*v.z,
                       m[8]*v.x + m[9]*v.y + m[10]*v.z);
}

__forceinline__ __device__ float4 TransformVector4x4(const float* m, const float4 &v) {
    return make_float4(m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3]*v.w,
                       m[4]*v.x + m[5]*v.y + m[6]*v.z + m[7]*v.w,
                       m[8]*v.x + m[9]*v.y + m[10]*v.z + m[11]*v.w,
                       m[12]*v.x + m[13]*v.y + m[14]*v.z + m[15]*v.w);
}

__forceinline__ __device__ float4 FloatMultiplyFloat4(const float n, const float4 &v) {
    return make_float4(n * v.x, n * v.y, n * v.z, 1.0f);
}

__forceinline__ __device__ float4 Float4MultiplyFloat4(const float4 &u, const float4 &v) {
    return make_float4(u.x * v.x, u.y * v.y, u.z * v.z, 1.0f);
}

__forceinline__ __device__ float3 Normalized(const float3 &u) {
    float length = sqrtf(u.x * u.x + u.y * u.y + u.z * u.z);
    return make_float3(u.x / length, u.y / length, u.z / length);
}

__forceinline__ __device__ float3 Float3Add(const float3 &u, const float3 &v) {
    return make_float3(u.x + v.x, u.y + v.y, u.z + v.z);
}

__forceinline__ __device__ float4 Float4Add(const float4 &u, const float4 &v) {
    return make_float4(u.x + v.x, u.y + v.y, u.z + v.z, 1.0f);
}

__forceinline__ __device__ float3 Float3Sub(const float3 &u, const float3 &v) {
    return make_float3(u.x - v.x, u.y - v.y, u.z - v.z);
}

__forceinline__ __device__ float4 Float4Sub(const float4 &u, const float4 &v) {
    return make_float4(u.x - v.x, u.y - v.y, u.z - v.z, 1.0f);
}

__forceinline__ __device__ float CudaDot(const float3 &u, const float3 &v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

__forceinline__ __device__ void ViewpointTransform(float4 &p, uint32_t screen_width, uint32_t screen_height) {
    float w = 1.0f / p.w;
    p.x = (p.x * w + 1.0f) * 0.5f * screen_width - 1e-12f;
    p.y = (p.y * w + 1.0f) * 0.5f * screen_height - 1e-12f;
}

__forceinline__ __device__ float2 BarycentricLerpFloat2(const float2 &v0, const float2 &v1, const float2 &v2, const float3 &g, float z_A, float z_B, float z_C) {
    float z = 1.0f / (g.x / z_A + g.y / z_B + g.z / z_C);
    // printf("BarycentricLerpFloat2 :: v0(%.3f, %.3f), v1(%.3f, %.3f), v2(%.3f, %.3f), g(%.3f, %.3f), z_A: %.3f, z_B: %.3f, z_C: %.3f, z: %.3f, res(%.3f %.3f)\n", v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, g.x, g.y, z_A, z_B, z_C, z, (v0.x * g.x / z_A + v1.x * g.y / z_B + v2.x * g.z / z_C) * z, (v0.y * g.x / z_A + v1.y * g.y / z_B + v2.y * g.z / z_C) * z);
    return make_float2((v0.x * g.x / z_A + v1.x * g.y / z_B + v2.x * g.z / z_C) * z,
                       (v0.y * g.x / z_A + v1.y * g.y / z_B + v2.y * g.z / z_C) * z);
}

__forceinline__ __device__ float3 BarycentricLerpFloat3(const float3 &v0, const float3 &v1, const float3 &v2, const float3 &g, float z_A, float z_B, float z_C) {
    float z = 1.0f / (g.x / z_A + g.y / z_B + g.z / z_C);
    return make_float3((v0.x * g.x / z_A + v1.x * g.y / z_B + v2.x * g.z / z_C) * z,
                       (v0.y * g.x / z_A + v1.y * g.y / z_B + v2.y * g.z / z_C) * z,
                       (v0.z * g.x / z_A + v1.z * g.y / z_B + v2.z * g.z / z_C) * z);
}

__forceinline__ __device__ float3 CudaCenterOfGravity(const float3 &a, const float3 &b, const float3 &c, const float2 &p) {
    if ((a.y - c.y) * b.x + (c.x - a.x) * b.y + (a.x * c.y - c.x * a.y) == 0) {
        return make_float3(1, 0, 0);
    }
    if ((a.y - b.y) * c.x + (b.x - a.x) * c.y + (a.x * b.y - b.x * a.y) == 0) {
        return make_float3(1, 0, 0);
    }
    float beta = fabsf((a.y - c.y) * p.x + (c.x - a.x) * p.y + (a.x * c.y - c.x * a.y)) /
                 fabsf((a.y - c.y) * b.x + (c.x - a.x) * b.y + (a.x * c.y - c.x * a.y));
    float gamma = fabsf((a.y - b.y) * p.x + (b.x - a.x) * p.y + (a.x * b.y - b.x * a.y)) /
                  fabsf((a.y - b.y) * c.x + (b.x - a.x) * c.y + (a.x * b.y - b.x * a.y));
    float alpha = 1.0f - beta - gamma;
    // printf("---centerofgravity--- ::  a(%.3f, %.3f, %.3f), b(%.3f, %.3f, %.3f), c(%.3f, %.3f, %.3f), p(%.3f, %.3f)\n", a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z, p.x, p.y);
    // printf("---centerofgravity--- :: alpha: %.3f, beta: %.3f, gamma: %.3f\n", alpha, beta, gamma);
    return make_float3(alpha, beta, gamma);
}

__forceinline__ __device__ float3 Float4ToFloat3(const float4 &v) {
    return make_float3(v.x, v.y, v.z);
}

__forceinline__ __device__ float4 Float3ToFloat4(const float3 &v) {
    return make_float4(v.x, v.y, v.z, 1.0f);
}


__forceinline__ __device__ float CudaMax(const float &a, const float &b) {
    return a > b ? a : b;
}

__forceinline__ __device__ float CudaMin(const float &a, const float &b) {
    return a < b ? a : b;
}

template<typename T>
__forceinline__ __device__ T CudaClamp(const T &min, const T &max, const T &value) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


__forceinline__ __device__ bool CudaZTestAndWrite(int x, int y, float depth, float *zbuffer, uint32_t screen_width, uint32_t screen_height, int *zbuffer_mutex) {
    bool blocked = true;
    bool is_ok = false;

    if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
        int _y = CudaClamp<int>(0, screen_height - 1, y);
        int _x = CudaClamp<int>(0, screen_width - 1, x);
        int idx = _y * screen_width + _x;

        while (blocked) {
            if (atomicCAS(&zbuffer_mutex[idx], 0, 1) == 0) {
                if (depth < zbuffer[idx]) {
                    atomicExch(&zbuffer[idx], depth);
                    is_ok = true;
                }
                __threadfence();
                atomicExch(&zbuffer_mutex[idx], 0);
                blocked = false;
            }
        }
    }
    return is_ok;
}



__forceinline__ __device__ float CudaCalculateShadow(float4 pos_light_space, float bias, const float *depth_buffer, uint32_t depth_buffer_width, uint32_t depth_buffer_height) {
    float w = 1.0f / pos_light_space.w;
    float p_x = (pos_light_space.x * w + 1.0f) * 0.5f * depth_buffer_width - 1e-12f;
    float p_y = (pos_light_space.y * w + 1.0f) * 0.5f * depth_buffer_height - 1e-12f;

    float depth = (pos_light_space.z + 1.0f) / 2.0f;

    float shadow = 0.0f;
    for (int i = -4; i <= 4; i ++) {
        for (int j = -4; j <= 4; j ++) {
            int y = (int)p_y + j;
            int x = (int)p_x + i;
            if (y < 0) y = 0;
            if (y >= depth_buffer_height) y = depth_buffer_height - 1;
            if (x < 0) x = 0;
            if (x >= depth_buffer_width) x = depth_buffer_width - 1;
            float depthbuffer_depth = depth_buffer[y * depth_buffer_width + x];

            shadow += depth > depthbuffer_depth + bias ? 1.0f : 0.0f;
        }
    }
    shadow /= 81.0f;
    // if (shadow > 0.9f) {
    //     printf("CudaCalculateShadow: lp() %.3f\n")
    // }

    return shadow;
}

__forceinline__ __device__ void AtomicExchFloat2(float2 &dest, const float2 &value) {
    atomicExch(&dest.x, value.x);
    atomicExch(&dest.y, value.y);
}

__forceinline__ __device__ void AtomicExchFloat3(float3 &dest, const float3 &value) {
    atomicExch(&dest.x, value.x);
    atomicExch(&dest.y, value.y);
    atomicExch(&dest.z, value.z);
}

__forceinline__ __device__ void AtomicExchFloat4(float4 &dest, const float4 &value) {
    atomicExch(&dest.x, value.x);
    atomicExch(&dest.y, value.y);
    atomicExch(&dest.z, value.z);
    atomicExch(&dest.w, value.w);
}

// template<typename T>
// __forceinline__ __device__ void AtomicExchPointer(T *dest, const T* value) {
//     atomicExch((unsigned long long int*)&dest, (unsigned long long int)value);
// }

#define CHECK(call) \
{ \
    const cudaError_t error=call; \
    if(error!=cudaSuccess) { \
        printf("ERROR: %s:%d, ", __FILE__, __LINE__); \
        printf("code: %d, reason: %s\n",error,cudaGetErrorString(error)); \
        exit(1); \
    } \
}

} // namespace lxrr_cuda