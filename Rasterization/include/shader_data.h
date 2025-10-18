#pragma once
#include <vector>
#include <memory>
#include "math_fun.h"
#include "vertex.h"
#include "vector.h"
#include "matrix.h"
#include "buffer.h"
#include "light.h"
#include "texture.h"
#include "material.h"

namespace lxrr {

/////////////////////////////////////////////////////////////////////////
////////////////////////////// Vert
/////////////////////////////////////////////////////////////////////////
struct VertUniform {
    mat4x4 m, v, p;
    vec3 camera_pos;
    
    std::shared_ptr<std::vector<std::shared_ptr<DirectionLight>>> direction_light_list;
    std::shared_ptr<std::vector<std::shared_ptr<PointLight>>> point_light_list;
    mat4x4 light_V, light_P;
};

struct VertIn {
    std::shared_ptr<Vertex> vertex;
    std::shared_ptr<vec4> light_vertex_pos; // vertex is in light space
};

struct VertOut {
    std::shared_ptr<Vertex> vertex;
    std::shared_ptr<vec4> light_vertex_pos;
    
    std::shared_ptr<vec3> world_pos;
    std::shared_ptr<vec3> world_normal;
    std::shared_ptr<vec3> view_pos;
};


/////////////////////////////////////////////////////////////////////////
////////////////////////////// Geometry
/////////////////////////////////////////////////////////////////////////
struct GeometryUniform {
};

struct GeometryIn {
    std::shared_ptr<std::vector<std::shared_ptr<Vertex>>> vertex_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec4>>> light_vertex_pos_list;

    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> world_pos_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> world_normal_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> view_pos_list;
};

struct GeometryOut {
    std::shared_ptr<std::vector<std::shared_ptr<Vertex>>> vertex_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec4>>> light_vertex_pos_list;

    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> world_pos_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> world_normal_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> view_pos_list;
};


/////////////////////////////////////////////////////////////////////////
////////////////////////////// Rasterization
/////////////////////////////////////////////////////////////////////////
struct RasterizationUniform {
    unsigned int screen_width, screen_height;
    std::shared_ptr<DepthBuffer> zbuffer;
};

struct RasterizationIn {
    std::shared_ptr<std::vector<std::shared_ptr<Vertex>>> vertex_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec4>>> light_vertex_pos_list;

    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> world_pos_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> world_normal_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> view_pos_list;
};

struct RasterizationOut {
    std::shared_ptr<std::vector<std::shared_ptr<vec2>>> frag_coord_list;
    std::shared_ptr<std::vector<bool>> frag_visible_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> frag_barycentric_coord_list;
    std::shared_ptr<std::vector<std::shared_ptr<color4>>> frag_color_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec2>>> frag_uv_list;
     
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> frag_world_pos_list;
    std::shared_ptr<std::vector<std::shared_ptr<vec3>>> frag_world_normal_list;

    std::shared_ptr<std::vector<std::shared_ptr<PhongMaterial>>> frag_material_list;
    std::shared_ptr<std::vector<std::shared_ptr<Texture>>> frag_texture_list;

    std::shared_ptr<std::vector<std::shared_ptr<vec4>>> lp0_list, lp1_list, lp2_list;
};


/////////////////////////////////////////////////////////////////////////
////////////////////////////// Frag
/////////////////////////////////////////////////////////////////////////
struct FragUniform {
    std::shared_ptr<DepthBuffer> depth_buffer;
};

struct FragIn {
    std::shared_ptr<vec2> frag_coord;
    std::shared_ptr<vec3> frag_barycentric_coord;
    std::shared_ptr<color4> frag_color;
    std::shared_ptr<vec2> frag_uv;
    
    std::shared_ptr<vec3> frag_world_pos;
    std::shared_ptr<vec3> frag_world_normal;

    std::shared_ptr<PhongMaterial> frag_material;
    std::shared_ptr<Texture> frag_texture;

    std::shared_ptr<vec4> lp0, lp1, lp2;
};

struct FragOut {
    std::shared_ptr<color4> color;
};

} // namespace lxrr