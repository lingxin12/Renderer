#include "renderer.h"
#include "shader_cuda.h"
#include "stb/stb_image_write.h"

namespace lxrr {

template<RenderType render_type>
bool Renderer::Render(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, std::shared_ptr<DepthBuffer> zbuffer) {
    //////////////////// Vert Shader
    auto VS_start_time = std::chrono::high_resolution_clock::now();

    auto vertex_list = std::make_shared<std::vector<std::shared_ptr<Vertex>>>();
    for (auto &vertex: *mesh->GetVertexs()) {
        vertex_list->push_back(std::make_shared<Vertex>(*vertex));
    }
    auto light_vertex_pos_list = std::make_shared<std::vector<std::shared_ptr<vec4>>>();

    for (auto &vertex: *vertex_list) {
        light_vertex_pos_list->push_back(std::make_shared<vec4>(vertex->pos));
    }

    auto world_pos_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();
    auto world_normal_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();
    auto view_pos_list = std::make_shared<std::vector<std::shared_ptr<vec3>>>();

    if (render_type == RenderType::Phong_CPU || render_type == RenderType::Shadow_CPU || render_type == RenderType::Shadow_GPU) {
        for (auto [vertex, light_vertex_pos]: std::views::zip(*vertex_list, *light_vertex_pos_list)) {
            VertIn vert_in = {
                vertex,
                light_vertex_pos
            };
            VertOut vert_out = shader->VS(vert_in);
            world_pos_list->push_back(vert_out.world_pos);
            world_normal_list->push_back(vert_out.world_normal);
            view_pos_list->push_back(vert_out.view_pos);
        }
    } else if (render_type == RenderType::Phong_GPU) {
        uint32_t N = vertex_list->size();
        std::shared_ptr<VertIn[]> vert_ins = std::make_shared<VertIn[]>(N);
        std::shared_ptr<VertOut[]> vert_outs = std::make_shared<VertOut[]>(N);
        for (size_t i = 0; i < N; i ++) {
            vert_ins[i] = {
                vertex_list->at(i),
                light_vertex_pos_list->at(i)
            };
            vert_outs[i] = {
                std::make_shared<Vertex>(*vertex_list->at(i)),
                std::make_shared<vec4>(*light_vertex_pos_list->at(i)),
                std::make_shared<vec3>(0, 0, 0),
                std::make_shared<vec3>(0, 0, 0),
                std::make_shared<vec3>(0, 0, 0)
            };
        }
        lxrr_cuda::CudaPhongShaderVS(N, vert_ins.get(), shader->GetVertUniform(), vert_outs.get());
        for (size_t i = 0; i < N; i ++) {
            vertex_list->at(i) = vert_outs[i].vertex;
            light_vertex_pos_list->at(i) = vert_outs[i].light_vertex_pos;
            world_pos_list->push_back(vert_outs[i].world_pos);
            world_normal_list->push_back(vert_outs[i].world_normal);
            view_pos_list->push_back(vert_outs[i].view_pos);
        }
    }
    std::cout << "VS end" << std::endl;
    // std::cout << "----------- output VSOut vertex_list ------------" << std::endl;
    // for (auto &v: *vertex_list) {
    //     std::cout << v->pos << std::endl;
    // }

    // auto vertexs = vertex_list;
    // // std::cout << "vertexs size : " << vertexs->size() << std::endl;
    // // std::cout << *vertexs->at(0) << std::endl;
    // // std::cout << *vertexs->at(2) << std::endl;
    // float max_x = std::numeric_limits<float>::lowest();
    // float max_y = std::numeric_limits<float>::lowest();
    // float max_z = std::numeric_limits<float>::lowest();
    // float min_x = std::numeric_limits<float>::max();
    // float min_y = std::numeric_limits<float>::max();
    // float min_z = std::numeric_limits<float>::max();
    // int max_i = 0;
    // for (int i = 0; i < vertexs->size(); i ++) {
    //     max_x = std::fmax(vertexs->at(i)->pos.x(), max_x);
    //     // if (vertexs->at(i)->pos.x() > max_x) {
    //     //     max_x = vertexs->at(i)->pos.x();
    //     //     max_i = i;
    //     // }
    //     max_y = std::fmax(vertexs->at(i)->pos.y(), max_y);
    //     max_z = std::fmax(vertexs->at(i)->pos.z(), max_z);
    //     min_x = std::fmin(vertexs->at(i)->pos.x(), min_x);
    //     min_y = std::fmin(vertexs->at(i)->pos.y(), min_y);
    //     min_z = std::fmin(vertexs->at(i)->pos.z(), min_z);
    // } 
    // std::cout << " aabb : " << min_x << " " << min_y << " " << min_z << " " << max_x << " " << max_y << " " << max_z << std::endl;
    // std::cout << "----------- output VSOut vertex_list ------------" << std::endl;

    std::cout << "Shader::VS runing time: " << calculate_time_span(VS_start_time) << std::endl;


    auto GS_start_time = std::chrono::high_resolution_clock::now();

    //////////////////// Geometry Shader
    GeometryIn geometry_in = {
        vertex_list,
        light_vertex_pos_list,
        world_pos_list,
        world_normal_list,
        view_pos_list
    };
    GeometryOut geometry_out = shader->GS(geometry_in);
    // std::cout << "GS end" << std::endl;

    // std::cout << "----------- output GSOut vertex_list ------------" << std::endl;
    // for (auto &v: *geometry_out.vertex_list) {
    //     std::cout << v->pos << std::endl;
    // }
    // for (auto &v: *geometry_out.light_vertex_pos_list) {
    //     std::cout << *v << std::endl;
    // }
    // for (auto &v: *geometry_out.vertex_list) {
    //     std::cout << v->uv << std::endl;
    // }
    // std::cout << "----------- output GSOut vertex_list ------------" << std::endl;

    std::cout << "Shader::GS runing time: " << calculate_time_span(GS_start_time) << std::endl;

    auto RS_start_time = std::chrono::high_resolution_clock::now();

    //////////////////// Rasterization Shader
    RasterizationIn rasterization_in = {
        geometry_out.vertex_list,
        geometry_out.light_vertex_pos_list,
        geometry_out.world_pos_list,
        geometry_out.world_normal_list,
        geometry_out.view_pos_list
    }; 
    RasterizationOut rasterization_out;

    if (render_type == RenderType::Phong_CPU || render_type == RenderType::Shadow_CPU || render_type == RenderType::Shadow_GPU) {
        // std::cout << "wo000" << std::endl;
        rasterization_out = shader->RS(rasterization_in);
    } else if (render_type == RenderType::Phong_GPU) {
        // std::cout << "wo111" << std::endl;
        shader->RasterizationOutInitialize(rasterization_out);
        lxrr_cuda::CudaPhongShaderRS(rasterization_in, shader->GetRasterizationUniform(), &rasterization_out);
    }

    std::cout << "RS end" << std::endl;

    std::cout << "Shader::RS runing time: " << calculate_time_span(RS_start_time) << std::endl;

    auto FS_start_time = std::chrono::high_resolution_clock::now();

    //////////////////// Frag Shader
    color_list = std::make_shared<std::vector<std::shared_ptr<color4>>>();
    for (int j = 0; j < zbuffer->GetHeight(); j ++) {
        for (int i = 0; i < zbuffer->GetWidth(); i ++) {
            color4 color = Color::black;
            color_list->push_back(std::make_shared<color4>(color));
        }
    }
    if (render_type == RenderType::Phong_CPU || render_type == RenderType::Shadow_CPU || render_type == RenderType::Shadow_GPU) { 
        // std::cout << "Shader::FS for start" << std::endl;
        for(auto [frag_coord,
                frag_visible,
                frag_barycentric_coord,
                frag_color,
                frag_uv,
                
                frag_world_pos,
                frag_world_normal,
                
                frag_material,
                frag_texture,
                
                frag_lps]:
            std::views::zip(
                *rasterization_out.frag_coord_list,
                *rasterization_out.frag_visible_list,
                *rasterization_out.frag_barycentric_coord_list,
                *rasterization_out.frag_color_list,
                *rasterization_out.frag_uv_list,
                
                *rasterization_out.frag_world_pos_list,
                *rasterization_out.frag_world_normal_list,
                
                *rasterization_out.frag_material_list,
                *rasterization_out.frag_texture_list,

                std::views::zip(*rasterization_out.lp0_list, *rasterization_out.lp1_list, *rasterization_out.lp2_list))) {
            
            if (!frag_visible) {
                continue;
            }

            auto [lp0, lp1, lp2] = frag_lps;

            FragIn frag_in = {
                frag_coord,
                frag_barycentric_coord,
                frag_color,
                frag_uv,
                
                frag_world_pos,
                frag_world_normal,

                frag_material,
                frag_texture,

                lp0, lp1, lp2
            };

            FragOut frag_out = shader->FS(frag_in);
            auto _i = frag_coord->x(), _j = frag_coord->y();
            (*color_list)[_j * zbuffer->GetWidth() + _i] = frag_out.color;
        }
    } else if (render_type == RenderType::Phong_GPU) {
        uint32_t M = zbuffer->GetWidth() * zbuffer->GetHeight();
        auto frag_in = std::make_shared<FragIn[]>(M);
        auto frag_out = std::make_shared<FragOut[]>(M);
        for (size_t i = 0; i < rasterization_out.frag_coord_list->size(); i ++) {
            frag_in[i] = {
                rasterization_out.frag_coord_list->at(i),
                rasterization_out.frag_barycentric_coord_list->at(i),
                rasterization_out.frag_color_list->at(i),
                rasterization_out.frag_uv_list->at(i),
                
                rasterization_out.frag_world_pos_list->at(i),
                rasterization_out.frag_world_normal_list->at(i),
                
                rasterization_out.frag_material_list->at(i),
                rasterization_out.frag_texture_list->at(i),

                rasterization_out.lp0_list->at(i),
                rasterization_out.lp1_list->at(i),
                rasterization_out.lp2_list->at(i)
            };
            frag_out[i] = {
                color_list->at(i)
            };
        }
        // std::cout << "CPU  :  before lxrr_cuda::CudaPhongShaderFS" << std::endl;
        lxrr_cuda::CudaPhongShaderFS(zbuffer->GetWidth(), zbuffer->GetHeight(), *rasterization_out.frag_visible_list, frag_in.get(), shader->GetFragUniform(), shader->GetVertUniform(), frag_out.get());
    }
    
    std::cout << "Shader::FS runing time: " << calculate_time_span(FS_start_time) << std::endl;

    pixel_coord_list = rasterization_out.frag_coord_list;

    // std::cout << "FS end " << pixel_coord_list->size() << " " << color_list->size() << std::endl;

    return true;
}

bool Renderer::DrawImage(const char* path) {
    if (pixel_coord_list == nullptr || color_list == nullptr || color_list->size() != pixel_coord_list->size()) {
        return false;
    }
    
    auto image_data = std::make_shared<unsigned char[]>(screen_width * screen_height * 4);
    
    for (int j = 0; j < screen_height; j ++) {
        for (int i = 0; i < screen_width; i ++){
            color4 color = *(*color_list)[j * screen_width + i];

            int index = ((screen_height - j - 1) * screen_width + (screen_width - i - 1)) * 4; ////////
            image_data[index + 0] = static_cast<unsigned char>(color[0] * 255.99f);
            image_data[index + 1] = static_cast<unsigned char>(color[1] * 255.99f);
            image_data[index + 2] = static_cast<unsigned char>(color[2] * 255.99f);
            image_data[index + 3] = static_cast<unsigned char>(color[3] * 255.99f);
        }
    }

    int result = stbi_write_png(path, screen_width, screen_height, 4, image_data.get(), screen_width * 4);
    
    // if (result) {
    //     std::cout << "Success to write image to " << path << "!" << std::endl;
    // } else {
    //     std::cout << "Failed to write image!" << std::endl;
    // }

    return true;
}

template bool Renderer::Render<RenderType::Phong_CPU>(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, std::shared_ptr<DepthBuffer> zbuffer);
template bool Renderer::Render<RenderType::Phong_GPU>(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, std::shared_ptr<DepthBuffer> zbuffer);
template bool Renderer::Render<RenderType::Shadow_CPU>(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, std::shared_ptr<DepthBuffer> zbuffer);
template bool Renderer::Render<RenderType::Shadow_GPU>(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, std::shared_ptr<DepthBuffer> zbuffer);

} // namespace lxrr