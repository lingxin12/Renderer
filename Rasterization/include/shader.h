#pragma once
#include <algorithm>
#include <functional>
#include <chrono>
#include "math_fun.h"
#include "shader_data.h"
#include "material.h"
#include "lighting_model.h"
#include "draw_impl.h"

namespace lxrr {

class Shader
{
public:
    virtual VertOut VS(VertIn &in) = 0;
    virtual GeometryOut GS(GeometryIn &in) = 0;
    virtual RasterizationOut RS(RasterizationIn &in) = 0;
    virtual FragOut FS(FragIn &in) = 0;

    virtual VertUniform& GetVertUniform() = 0;
    virtual GeometryUniform& GetGeometryUniform() = 0;
    virtual RasterizationUniform& GetRasterizationUniform() = 0;
    virtual FragUniform& GetFragUniform() = 0;

    virtual void RasterizationOutInitialize(RasterizationOut &out) = 0;
};

class PhongShader: public Shader
{
public:
    PhongShader() = default;
    PhongShader(const VertUniform &vu, const GeometryUniform &gu, const RasterizationUniform &ru, const FragUniform &fu, bool use_cuda = false):
        vert_uniform(vu), geometry_uniform(gu), rasterization_uniform(ru), frag_uniform(fu), use_cuda(use_cuda) {}
    
    virtual VertOut VS(VertIn &in) override;
    virtual GeometryOut GS(GeometryIn &in) override;
    virtual RasterizationOut RS(RasterizationIn &in) override;
    virtual FragOut FS(FragIn &in) override;

    VertUniform& GetVertUniform() { return vert_uniform; }
    GeometryUniform& GetGeometryUniform() { return geometry_uniform; }
    RasterizationUniform& GetRasterizationUniform() { return rasterization_uniform; }
    FragUniform& GetFragUniform() { return frag_uniform; }

    void RasterizationOutInitialize(RasterizationOut &out);


private:
    VertUniform vert_uniform;
    GeometryUniform geometry_uniform;
    RasterizationUniform rasterization_uniform;
    FragUniform frag_uniform;

    bool use_cuda;
};

class ShadowShader: public Shader
{
public:
    ShadowShader() = default;
    ShadowShader(const VertUniform &vu, const GeometryUniform &gu, const RasterizationUniform &ru, const FragUniform &fu, bool use_cuda = false):
        vert_uniform(vu), geometry_uniform(gu), rasterization_uniform(ru), frag_uniform(fu), use_cuda(use_cuda) {}

    virtual VertOut VS(VertIn &in) override;
    virtual GeometryOut GS(GeometryIn &in) override;
    virtual RasterizationOut RS(RasterizationIn &in) override;
    virtual FragOut FS(FragIn &in) override;

    VertUniform& GetVertUniform() { return vert_uniform; }
    GeometryUniform& GetGeometryUniform() { return geometry_uniform; }
    RasterizationUniform& GetRasterizationUniform() { return rasterization_uniform; }
    FragUniform& GetFragUniform() { return frag_uniform; }

    void RasterizationOutInitialize(RasterizationOut &out);

private:
    VertUniform vert_uniform;
    GeometryUniform geometry_uniform;
    RasterizationUniform rasterization_uniform;
    FragUniform frag_uniform;

    bool use_cuda;
};

} // namespace lxrr