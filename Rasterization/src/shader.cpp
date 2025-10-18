#include "shader.h"

namespace lxrr {

void Shader::RasterizationOutInitialize(RasterizationOut &out) {
    
}

VertOut PhongShader::VS(VertIn &in) {
    // std::cout << "-------------------------------- PhongShader::VS Debug --------------" << std::endl << std::endl << std::endl;

    auto v = in.vertex;
    auto lp = in.light_vertex_pos;
    auto uniform = vert_uniform;

    *lp = uniform.m * (*lp);
    *lp = uniform.light_V * (*lp);
    *lp = uniform.light_P * (*lp);

    vec4 old_p = v->pos;

    // std::cout << "before vertex pos : " << v->pos << std::endl;
    v->pos = uniform.m * v->pos;
    auto world_pos = std::make_shared<vec3>(v->pos.get_3d());
    v->pos = uniform.v * v->pos;
    auto view_pos = std::make_shared<vec3>(v->pos.get_3d());
    // std::cout << "view_pos : " << v->pos << std::endl;
    v->pos = uniform.p * v->pos;
    v->pos.standardization();
    // std::cout << "uniform.p" << uniform.p << std::endl;
    // std::cout << "after vertex pos : " << v->pos << std::endl;

    v->normal = (uniform.m.get_3x3() * v->normal).normalized();
    auto world_normal = std::make_shared<vec3>(v->normal);
    // std::cout << "world_normal : " << *world_normal << std::endl;

    VertOut out;
    out.vertex = v;
    out.light_vertex_pos = lp;
    out.world_pos = world_pos;
    out.world_normal = world_normal;
    out.view_pos = view_pos;

    // if (std::fabs(v->pos.x()) > 1000.0f || std::fabs(v->pos.y()) > 1000.0f || std::fabs(v->pos.z()) > 1000.0f) {
    //     std::cout << " test vertex :: " << old_p << " " << v->pos << std::endl;
    // }
    // std::cout << std::endl << std::endl << std::endl << "-------------------------------- PhongShader::VS Debug --------------" << std::endl;

    return out;
}

GeometryOut PhongShader::GS(GeometryIn &in) {
    auto vertex_list = in.vertex_list;
    auto light_vertex_pos_list = in.light_vertex_pos_list;
    auto world_pos_list = in.world_pos_list;
    auto world_normal_list = in.world_normal_list;
    auto view_pos_list = in.view_pos_list;

    for (int i = 0; i < in.vertex_list->size(); i += 3) {
        auto v0 = (*vertex_list)[i];
        auto v1 = (*vertex_list)[i+1];
        auto v2 = (*vertex_list)[i+2];
        auto lp0 = (*light_vertex_pos_list)[i];
        auto lp1 = (*light_vertex_pos_list)[i+1];
        auto lp2 = (*light_vertex_pos_list)[i+2];
        auto world_pos0 = (*world_pos_list)[i];
        auto world_pos1 = (*world_pos_list)[i+1];
        auto world_pos2 = (*world_pos_list)[i+2];
        auto world_normal0 = (*world_normal_list)[i];
        auto world_normal1 = (*world_normal_list)[i+1];
        auto world_normal2 = (*world_normal_list)[i+2];
        auto view_pos0 = (*view_pos_list)[i];
        auto view_pos1 = (*view_pos_list)[i+1];
        auto view_pos2 = (*view_pos_list)[i+2];
        
        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        if (v1->pos.y() < v0->pos.y()) {
            // std::cout << "swap (1, 0)" << std::endl;
            std::swap(v0, v1);
            std::swap(lp0, lp1);
            std::swap(world_pos0, world_pos1);
            std::swap(world_normal0, world_normal1);
            std::swap(view_pos0, view_pos1);
        }

        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        if (v2->pos.y() < v0->pos.y()) {
            // std::cout << "swap (0, 2)" << std::endl;
            std::swap(v0, v2);
            std::swap(lp0, lp2);
            std::swap(world_pos0, world_pos2);
            std::swap(world_normal0, world_normal2);
            std::swap(view_pos0, view_pos2);
        }

        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        if (v2->pos.y() < v1->pos.y()) {
            // std::cout << "swap (2, 1)" << std::endl;
            std::swap(v1, v2);
            std::swap(lp1, lp2);
            std::swap(world_pos1, world_pos2);
            std::swap(world_normal1, world_normal2);
            std::swap(view_pos1, view_pos2);
        }

        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        (*vertex_list)[i] = v0;
        (*vertex_list)[i+1] = v1;
        (*vertex_list)[i+2] = v2;
        (*light_vertex_pos_list)[i] = lp0;
        (*light_vertex_pos_list)[i+1] = lp1;
        (*light_vertex_pos_list)[i+2] = lp2;
        (*world_pos_list)[i] = world_pos0;
        (*world_pos_list)[i+1] = world_pos1;
        (*world_pos_list)[i+2] = world_pos2;
        (*world_normal_list)[i] = world_normal0;
        (*world_normal_list)[i+1] = world_normal1;
        (*world_normal_list)[i+2] = world_normal2;
        (*view_pos_list)[i] = view_pos0;
        (*view_pos_list)[i+1] = view_pos1;
        (*view_pos_list)[i+2] = view_pos2;
    }
    
    GeometryOut out;
    // CullCheck ......

    out.vertex_list = vertex_list;
    out.light_vertex_pos_list = light_vertex_pos_list;
    out.world_pos_list = world_pos_list;
    out.world_normal_list = world_normal_list;
    out.view_pos_list = view_pos_list;

    // for (auto &v: *out.vertex_list) {
    //     std::cout << v->pos << std::endl;
    // }

    return out;
}

RasterizationOut PhongShader::RS(RasterizationIn &in) {
    auto vertex_list = in.vertex_list;
    auto light_vertex_pos_list = in.light_vertex_pos_list;
    auto world_pos_list = in.world_pos_list;
    auto world_normal_list = in.world_normal_list;
    auto view_pos_list = in.view_pos_list;

    auto uniform = rasterization_uniform;

    RasterizationOut out;

    RasterizationOutInitialize(out);

    for (int i = 0; i < in.vertex_list->size(); i += 3) {
        // if (i >= 52800 && i <= 53100 && i % 3 == 0) {
        //     std::cout << "RS process " << i << "   all : " << in.vertex_list->size() << std::endl;
        // }
        auto rs_point_start_time = std::chrono::high_resolution_clock::now();

        auto v0 = (*vertex_list)[i];
        auto v1 = (*vertex_list)[i+1];
        auto v2 = (*vertex_list)[i+2];
        auto lp0 = (*light_vertex_pos_list)[i];
        auto lp1 = (*light_vertex_pos_list)[i+1];
        auto lp2 = (*light_vertex_pos_list)[i+2];
        auto wp0 = (*world_pos_list)[i];
        auto wp1 = (*world_pos_list)[i+1];
        auto wp2 = (*world_pos_list)[i+2];
        auto wn0 = (*world_normal_list)[i];
        auto wn1 = (*world_normal_list)[i+1];
        auto wn2 = (*world_normal_list)[i+2];
        auto vp0 = (*view_pos_list)[i];
        auto vp1 = (*view_pos_list)[i+1];
        auto vp2 = (*view_pos_list)[i+2];

        // std::cout << "ras tri ----- " << rasterization_uniform.zbuffer->Get(13, 36) << std::endl;

        // CullCheck!!!!
    
        ScanlineRender::ViewpointTransform(*v0, uniform.screen_width, uniform.screen_height);
        ScanlineRender::ViewpointTransform(*v1, uniform.screen_width, uniform.screen_height);
        ScanlineRender::ViewpointTransform(*v2, uniform.screen_width, uniform.screen_height);

        // 3 point overlap
        if (((int)v0->pos.y() == (int)v1->pos.y() && abs(v0->pos.y() - v2->pos.y()) <= 1) ||
            ((int)v1->pos.y() == (int)v2->pos.y() && abs(v2->pos.y() - v0->pos.y()) <= 1) ||
            ((int)v0->pos.y() == (int)v2->pos.y() && abs(v1->pos.y() - v2->pos.y()) <= 1)) {
            continue;
        }

        // std::cout << "viewpoint pos:  " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        ScanlineRender::RasterizeTriangle(*v0, *v1, *v2, uniform.zbuffer, out, *lp0, *lp1, *lp2, *wp0, *wp1, *wp2, *wn0, *wn1, *wn2, *vp0, *vp1, *vp2);

        // std::cout << "Shader::RS rs_point_start_time: " << i << " " << *v0 << " " << *v1 << " " << *v2 << " " << calculate_time_span(rs_point_start_time) << std::endl;
    }

    return out;
}

void PhongShader::RasterizationOutInitialize(RasterizationOut &out) {
    out.frag_coord_list = std::make_shared<std::vector<std::shared_ptr<vec2>>>();
    out.frag_visible_list = std::make_shared<std::vector<bool>>();
    out.frag_barycentric_coord_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();
    out.frag_color_list = std::make_shared<std::vector<std::shared_ptr<color4>>>();
    out.frag_uv_list = std::make_shared<std::vector<std::shared_ptr<vec2>>>();

    out.frag_world_pos_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();
    out.frag_world_normal_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();

    out.frag_material_list = std::make_shared<std::vector<std::shared_ptr<PhongMaterial>>>();
    out.frag_texture_list = std::make_shared<std::vector<std::shared_ptr<Texture>>>();

    out.lp0_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();
    out.lp1_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();
    out.lp2_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();


    for (int j = 0; j < rasterization_uniform.screen_height; j ++) {
        for (int i = 0; i < rasterization_uniform.screen_width; i ++) {
            out.frag_coord_list->push_back(std::make_shared<vec2>(i, j));
            out.frag_visible_list->push_back(false);
            out.frag_barycentric_coord_list->push_back(std::make_shared<vec3>(1, 0, 0));
            out.frag_color_list->push_back(std::make_shared<color4>(0,0,0,1));
            out.frag_uv_list->push_back(std::make_shared<vec2>(0,0));

            out.frag_world_pos_list->push_back(std::make_shared<vec3>(0,0,0));
            out.frag_world_normal_list->push_back(std::make_shared<vec3>(0,0,0));

            out.frag_material_list->push_back(MaterialLibrary::GetInstance().GetMaterial("default"));
            out.frag_texture_list->push_back(nullptr);

            out.lp0_list->push_back(std::make_shared<vec4>(0,0,0));
            out.lp1_list->push_back(std::make_shared<vec4>(0,0,0));
            out.lp2_list->push_back(std::make_shared<vec4>(0,0,0));
        }
    }
}

FragOut PhongShader::FS(FragIn &in) {
    // std::cout << "-------------------------------- PhongShader::FS Debug --------------" << std::endl << std::endl;

    auto frag_coord = in.frag_coord;
    auto frag_barycentric_coord = in.frag_barycentric_coord;
    auto frag_color = in.frag_color;
    auto frag_uv = in.frag_uv;
    
    auto frag_world_pos = in.frag_world_pos;
    auto frag_world_normal = in.frag_world_normal;

    auto frag_material = in.frag_material;
    auto frag_texture = in.frag_texture;

    auto lp0 = in.lp0, lp1 = in.lp1, lp2 = in.lp2;

    auto zbuffer = frag_uniform.depth_buffer;
    auto view_pos = vert_uniform.camera_pos;
    auto direction_light_list = vert_uniform.direction_light_list;
    auto point_light_list = vert_uniform.point_light_list;

    // std::cout << " ------------------------ " << std::endl;
    // std::cout << "frag_coord: " << *frag_coord << std::endl;
    // std::cout << "CPU lp check --- " << *lp0 << " " << *lp1 << " " << *lp2 << std::endl;

    // std::cout << "bliin model" << std::endl;
    // bliinphong model
    color4 rgb;
    if (frag_texture == nullptr) {
        rgb = color4(frag_material->diffuse);
    } else {
        // std::cout << "Sample " << frag_uv->x() << "  " << frag_uv->y() << std::endl;
        rgb = frag_texture->Sample(frag_uv->x(), frag_uv->y());
    }

    // std::cout << "rgb: " << rgb << std::endl;

    color4 render_color = color4(0,0,0,1);

    // std::cout << "direction light" << std::endl;
    // direction light
    for (auto &light: *direction_light_list) {
        // ambient
        color4 ambient = light->get_ka() * rgb;
        // std::cout << "ambient: " << ambient << " " << light->get_ka() << " " << rgb << std::endl;
        // diffuse
        vec3 normal = frag_world_normal->normalized();
        // std::cout << "frag_world_normal: " << *frag_world_normal << std::endl;
        vec3 l_dir = (vec3(0,0,0) - light->get_dir()).normalized();
        // std::cout << "l_dir: " << l_dir << std::endl;
        color4 diffuse = light->get_kd() * light->get_color() * std::max(color4::dot(normal, l_dir), 0.0f) * rgb;
        // std::cout << "diffuse: " << diffuse << " " << normal << " " << l_dir << " " << std::max(color4::dot(normal, l_dir), 0.0f) << std::endl;
        // specular
        vec3 v_dir = (view_pos - *frag_world_pos).normalized();
        vec3 h = (v_dir + l_dir).normalized();
        color4 specular = light->get_ks() * light->get_color() * std::pow(std::max(color4::dot(normal, h), 0.0f), frag_material->shininess) * color4(frag_material->specular);
        // std::cout << "specular: " << specular << std::endl;

        render_color = render_color + ambient + diffuse + specular;
        // std::cout << "render_color: " << render_color << std::endl;
    }

    // std::cout << "------------------" << std::endl;

    // // point light
    // for (auto &light: *point_light_list) {
    //     // ambient
    //     color4 ambient = light->get_ka() * light->get_color() * rgb;
        
    //     // diffuse
    //     vec3 normal = frag_world_normal->normalized();
    //     vec3 l_dir = (light->get_pos() - *frag_world_pos).normalized();
    //     color4 diffuse = light->get_kd() * light->get_color() * std::max(color4::dot(normal, l_dir), 0.0f) * rgb;

    //     // specular
    //     vec3 v_dir = (view_pos - *frag_world_pos).normalized();
    //     vec3 h = (v_dir + l_dir).normalized();
    //     color4 specular = light->get_ks() * light->get_color() * std::pow(std::max(color4::dot(normal, h), 0.0f), frag_material->shininess) * rgb;

    //     render_color = render_color + ambient + diffuse + specular;
    // }

    // std::cout << "shadow light" << std::endl;
    // shadow (single direction light)
    if (direction_light_list->size() > 0) {
        vec3 normal = frag_world_normal->normalized();
        vec3 l_dir = (vec3(0,0,0) - (*direction_light_list)[0]->get_dir()).normalized();

        float bias = std::max(0.02f * (1.0f - std::abs(vec3::dot(normal, l_dir))), 0.005f);

        vec4 frag_pos_on_light_space = *lp0 * frag_barycentric_coord->x() + *lp1 * frag_barycentric_coord->y() + *lp2 * frag_barycentric_coord->z();

        float shadow = ScanlineRender::CalculateShadow(frag_pos_on_light_space, bias, frag_uniform.depth_buffer);
        // std::cout << "FS shadow: " << shadow << " " << bias << " " << frag_pos_on_light_space << " " << *lp0 << " " << *lp1 << " " << *lp2 << " " << frag_barycentric_coord->x() << " " << frag_barycentric_coord->y() << " " << frag_barycentric_coord->z() << std::endl;
        // if (shadow > 0.5f) {
        //     std::cout << "frag_coord: " << *frag_coord << "   shadow is so large  " << shadow << std::endl; 
        // }
        // std::cout << "frag_coord: " << *frag_coord << "   shadow is so large  " << shadow << std::endl;
        // render_color = render_color * (1 - shadow);
    }
    // std::cout << std::endl;

    FragOut out;
    out.color = std::make_shared<color4>(render_color);

    // std::cout << std::endl << std::endl << "-------------------------------- PhongShader::FS Debug --------------" << std::endl;

    return out;
}











VertOut ShadowShader::VS(VertIn &in) {
    auto v = in.vertex;
    auto lp = in.light_vertex_pos;
    auto uniform = vert_uniform;

    v->pos = uniform.m * v->pos;
    auto world_pos = std::make_shared<vec3>(v->pos.get_3d());
    v->pos = uniform.light_V * v->pos;
    auto view_pos = std::make_shared<vec3>(v->pos.get_3d());
    v->pos = uniform.light_P * v->pos;
    v->pos.standardization();

    v->normal = (uniform.m.get_3x3() * v->normal).normalized();
    auto world_normal = std::make_shared<vec3>(v->normal);

    VertOut out;
    out.vertex = v;
    out.light_vertex_pos = lp;
    out.world_pos = world_pos;
    out.world_normal = world_normal;
    out.view_pos = view_pos;

    return out;
}

GeometryOut ShadowShader::GS(GeometryIn &in) {
    auto vertex_list = in.vertex_list;
    auto light_vertex_pos_list = in.light_vertex_pos_list;
    auto world_pos_list = in.world_pos_list;
    auto world_normal_list = in.world_normal_list;
    auto view_pos_list = in.view_pos_list;

    for (int i = 0; i < in.vertex_list->size(); i += 3) {
        auto v0 = (*vertex_list)[i];
        auto v1 = (*vertex_list)[i+1];
        auto v2 = (*vertex_list)[i+2];
        auto lp0 = (*light_vertex_pos_list)[i];
        auto lp1 = (*light_vertex_pos_list)[i+1];
        auto lp2 = (*light_vertex_pos_list)[i+2];
        auto world_pos0 = (*world_pos_list)[i];
        auto world_pos1 = (*world_pos_list)[i+1];
        auto world_pos2 = (*world_pos_list)[i+2];
        auto world_normal0 = (*world_normal_list)[i];
        auto world_normal1 = (*world_normal_list)[i+1];
        auto world_normal2 = (*world_normal_list)[i+2];
        auto view_pos0 = (*view_pos_list)[i];
        auto view_pos1 = (*view_pos_list)[i+1];
        auto view_pos2 = (*view_pos_list)[i+2];
        
        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        if (v1->pos.y() < v0->pos.y()) {
            // std::cout << "swap (1, 0)" << std::endl;
            std::swap(v0, v1);
            std::swap(lp0, lp1);
            std::swap(world_pos0, world_pos1);
            std::swap(world_normal0, world_normal1);
            std::swap(view_pos0, view_pos1);
        }

        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        if (v2->pos.y() < v0->pos.y()) {
            // std::cout << "swap (0, 2)" << std::endl;
            std::swap(v0, v2);
            std::swap(lp0, lp2);
            std::swap(world_pos0, world_pos2);
            std::swap(world_normal0, world_normal2);
            std::swap(view_pos0, view_pos2);
        }

        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        if (v2->pos.y() < v1->pos.y()) {
            // std::cout << "swap (2, 1)" << std::endl;
            std::swap(v1, v2);
            std::swap(lp1, lp2);
            std::swap(world_pos1, world_pos2);
            std::swap(world_normal1, world_normal2);
            std::swap(view_pos1, view_pos2);
        }

        // std::cout << "GSSS : " << v0->pos << " " << v1->pos << " " << v2->pos << std::endl;

        (*vertex_list)[i] = v0;
        (*vertex_list)[i+1] = v1;
        (*vertex_list)[i+2] = v2;
        (*light_vertex_pos_list)[i] = lp0;
        (*light_vertex_pos_list)[i+1] = lp1;
        (*light_vertex_pos_list)[i+2] = lp2;
        (*world_pos_list)[i] = world_pos0;
        (*world_pos_list)[i+1] = world_pos1;
        (*world_pos_list)[i+2] = world_pos2;
        (*world_normal_list)[i] = world_normal0;
        (*world_normal_list)[i+1] = world_normal1;
        (*world_normal_list)[i+2] = world_normal2;
        (*view_pos_list)[i] = view_pos0;
        (*view_pos_list)[i+1] = view_pos1;
        (*view_pos_list)[i+2] = view_pos2;
    }
    
    GeometryOut out;
    // CullCheck ......

    out.vertex_list = vertex_list;
    out.light_vertex_pos_list = light_vertex_pos_list;
    out.world_pos_list = world_pos_list;
    out.world_normal_list = world_normal_list;
    out.view_pos_list = view_pos_list;

    // for (auto &v: *out.vertex_list) {
    //     std::cout << v->pos << std::endl;
    // }

    return out;
}

RasterizationOut ShadowShader::RS(RasterizationIn &in) {
    auto vertex_list = in.vertex_list;
    auto light_vertex_pos_list = in.light_vertex_pos_list;
    auto world_pos_list = in.world_pos_list;
    auto world_normal_list = in.world_normal_list;
    auto view_pos_list = in.view_pos_list;

    auto uniform = rasterization_uniform;

    RasterizationOut out;

    RasterizationOutInitialize(out);

    for (int i = 0; i < in.vertex_list->size(); i += 3) {
        auto v0 = (*vertex_list)[i];
        auto v1 = (*vertex_list)[i+1];
        auto v2 = (*vertex_list)[i+2];
        auto lp0 = (*light_vertex_pos_list)[i];
        auto lp1 = (*light_vertex_pos_list)[i+1];
        auto lp2 = (*light_vertex_pos_list)[i+2];
        auto wp0 = (*world_pos_list)[i];
        auto wp1 = (*world_pos_list)[i+1];
        auto wp2 = (*world_pos_list)[i+2];
        auto wn0 = (*world_normal_list)[i];
        auto wn1 = (*world_normal_list)[i+1];
        auto wn2 = (*world_normal_list)[i+2];
        auto vp0 = (*view_pos_list)[i];
        auto vp1 = (*view_pos_list)[i+1];
        auto vp2 = (*view_pos_list)[i+2];

        ScanlineRender::ViewpointTransform(*v0, uniform.screen_width, uniform.screen_height);
        ScanlineRender::ViewpointTransform(*v1, uniform.screen_width, uniform.screen_height);
        ScanlineRender::ViewpointTransform(*v2, uniform.screen_width, uniform.screen_height);

        // 3 point overlap
        if (((int)v0->pos.y() == (int)v1->pos.y() && abs(v0->pos.y() - v2->pos.y()) <= 1) ||
            ((int)v1->pos.y() == (int)v2->pos.y() && abs(v2->pos.y() - v0->pos.y()) <= 1) ||
            ((int)v0->pos.y() == (int)v2->pos.y() && abs(v1->pos.y() - v2->pos.y()) <= 1)) {
            continue;
        }

        ScanlineRender::RasterizeTriangle(*v0, *v1, *v2, uniform.zbuffer, out, *lp0, *lp1, *lp2, *wp0, *wp1, *wp2, *wn0, *wn1, *wn2, *vp0, *vp1, *vp2);
    }

    return out;
}

void ShadowShader::RasterizationOutInitialize(RasterizationOut &out) {
    out.frag_coord_list = std::make_shared<std::vector<std::shared_ptr<vec2>>>();
    out.frag_visible_list = std::make_shared<std::vector<bool>>();
    out.frag_barycentric_coord_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();
    out.frag_color_list = std::make_shared<std::vector<std::shared_ptr<color4>>>();
    out.frag_uv_list = std::make_shared<std::vector<std::shared_ptr<vec2>>>();

    out.frag_world_pos_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();
    out.frag_world_normal_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();

    out.frag_material_list = std::make_shared<std::vector<std::shared_ptr<PhongMaterial>>>();
    out.frag_texture_list = std::make_shared<std::vector<std::shared_ptr<Texture>>>();

    out.lp0_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();
    out.lp1_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();
    out.lp2_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();


    for (int j = 0; j < rasterization_uniform.screen_height; j ++) {
        for (int i = 0; i < rasterization_uniform.screen_width; i ++) {
            out.frag_coord_list->push_back(std::make_shared<vec2>(i, j));
            out.frag_visible_list->push_back(false);
            out.frag_barycentric_coord_list->push_back(std::make_shared<vec3>(1, 0, 0));
            out.frag_color_list->push_back(std::make_shared<color4>(0,0,0,1));
            out.frag_uv_list->push_back(std::make_shared<vec2>(0,0));

            out.frag_world_pos_list->push_back(std::make_shared<vec3>(0,0,0));
            out.frag_world_normal_list->push_back(std::make_shared<vec3>(0,0,0));

            out.frag_material_list->push_back(MaterialLibrary::GetInstance().GetMaterial("default"));
            out.frag_texture_list->push_back(nullptr);

            out.lp0_list->push_back(std::make_shared<vec4>(0,0,0));
            out.lp1_list->push_back(std::make_shared<vec4>(0,0,0));
            out.lp2_list->push_back(std::make_shared<vec4>(0,0,0));
        }
    }
}


FragOut ShadowShader::FS(FragIn &in) {
    FragOut out;
    return out;
}

} // namespace lxrr