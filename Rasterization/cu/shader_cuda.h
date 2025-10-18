#include <cstdio>
#include <iostream>
#include <vector>
#include <memory>
#include "shader.h"
#include "vertex.h"
#include "material.h"
#include "texture.h"

namespace lxrr_cuda {

void CudaPhongShaderVS(uint32_t N, const lxrr::VertIn *in, const lxrr::VertUniform &vert_uniform, lxrr::VertOut *out);

// void CudaPhongShaderGS(const lxrr::GeometryIn &in, const lxrr::GeometryUniform &geometry_uniform, lxrr::GeometryOut *out);

void CudaPhongShaderRS(const lxrr::RasterizationIn &in, const lxrr::RasterizationUniform &rasterization_uniform, lxrr::RasterizationOut *out);

void CudaPhongShaderFS(uint32_t screen_width, uint32_t screen_height, std::vector<bool> &frag_visible_list, const lxrr::FragIn *in, const lxrr::FragUniform &frag_uniform, lxrr::VertUniform &vert_uniform, lxrr::FragOut *out);

// void CudaShadowShaderVS(uint32_t N, const lxrr::VertIn *in, const lxrr::VertUniform &vert_uniform, lxrr::VertOut *out);

// void CudaShadowShaderRS(const lxrr::RasterizationIn &in, const lxrr::RasterizationUniform &rasterization_uniform, lxrr::RasterizationOut *out);

// void CudaShadowShaderFS(const lxrr::FragIn *in, const lxrr::FragUniform &frag_uniform, lxrr::FragOut *out);

// void CudaShadowShaderGS(const lxrr::GeometryIn &in, const lxrr::GeometryUniform &geometry_uniform, lxrr::GeometryOut *out);

} // lxrr_cuda