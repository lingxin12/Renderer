#include <iostream>
#include <memory>
#include <chrono>
#include <algorithm>
#include <cmath>
#include "shader.h"
#include "renderer.h"
#include "transform.h"
#include "read_obj.h"

using namespace std;
using namespace lxrr;

void CreateObject(Mesh&, std::shared_ptr<std::vector<std::shared_ptr<Texture>>>);


static unsigned int screen_width = 400, screen_height = 300;

int main(int argc, char* argv[])
{
    const char *output_path = "output.png";
    bool use_cuda = false;
    if (argc > 1) {
        if (argv[1] == "--cuda") {
            use_cuda = true;
        }
    }

    // camera
    vec3 camera_pos(0,0,0);
    float fov = 45.0f;
    float z_near = 1.0f, z_far = 100.0f;
    mat4x4 camera_M = mat4x4();
    camera_M.set_3x3(Transform::scale(vec3(2,2,2)) * camera_M.get_3x3());
    // camera_M.set_3x3(Transform::rotate(vec3(-40,0,0)) * camera_M.get_3x3());
    // camera_M = Transform::translate(vec3(-5,-5,-35)) * camera_M;
    camera_M.set_3x3(Transform::rotate(vec3(40,35,0)) * camera_M.get_3x3());
    camera_M = Transform::translate(vec3(0,0,-7)) * camera_M;
    mat4x4 camera_V = Camera::LookAt(camera_pos, vec3(0,0,-1), vec3(0,1,0));
    mat4x4 camera_P = Camera::Perspective(fov, (float)screen_width/screen_height, z_near, z_far);
    auto camera = std::make_shared<Camera>(camera_pos, vec3(0,0,0), vec3(1,1,1), camera_M, camera_V, camera_P);

    // light
    mat4x4 light_P = Camera::Orthographic(-20, 20, 0, 120, 20, -20);
    vec3 light_pos(0,0,1);
    vec3 light_dir(0,0,-1); 
    DirectionLight light(light_pos, light_dir, 1, color4(1,1,1));
    mat4x4 light_V = Camera::LookAt(light_pos, light_dir);

    auto direction_light_list = std::make_shared<std::vector<std::shared_ptr<DirectionLight>>>();
    auto point_light_list = std::make_shared<std::vector<std::shared_ptr<PointLight>>>();
    direction_light_list->push_back(std::make_shared<DirectionLight>(light));
    
    // texture
    Texture texture;
    texture.LoadTexture("./textures/container2.png");
    auto texture_list = std::make_shared<std::vector<std::shared_ptr<Texture>>>();
    texture_list->push_back(std::make_shared<Texture>(texture));

    // buffer
    auto depth_buffer = std::make_shared<DepthBuffer>(screen_width * 2, screen_height * 2);
    auto zbuffer = std::make_shared<DepthBuffer>(screen_width, screen_height);

    // std::cout << "before shader load" << std::endl;

    // shader
    VertUniform phong_vu = {
        camera->get_object_to_world(), camera->get_world_to_view(), camera->get_perspective_matrix(),
        camera->get_pos(),
        direction_light_list, point_light_list,
        light_V, light_P
    };
    GeometryUniform phong_gu;
    RasterizationUniform phong_ru = {
        screen_width, screen_height,
        zbuffer
    };
    FragUniform phong_fu = {
        depth_buffer
    };
    auto phong_shader = std::make_shared<PhongShader>(phong_vu, phong_gu, phong_ru, phong_fu);

    VertUniform shadow_vu = phong_vu;
    GeometryUniform shadow_gu;
    RasterizationUniform shadow_ru = {screen_width * 2, screen_height * 2, depth_buffer};
    FragUniform shadow_fu = phong_fu;
    auto shadow_shader = std::make_shared<ShadowShader>(shadow_vu, shadow_gu, shadow_ru, shadow_fu);

    // mesh
    auto mesh = std::make_shared<Mesh>();
    CreateObject(*mesh, texture_list);
    // auto mesh = ReadOBJ::Read("models/hand/16834_hand_v1_NEW.obj");

    // std::cout << "after shader load" << std::endl;

    // Render
    auto renderer = std::make_unique<Renderer>(
        screen_width, screen_height,
        camera,
        texture_list,
        direction_light_list,
        point_light_list,
        light_V, light_P,
        phong_shader
    );

    // std::cout << "before render" << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    // std::cout << "----------- before shadow render -----------" << std::endl;
    // for (auto &v: *mesh->GetVertexs()) {
    //     std::cout << v->pos << std::endl;
    // }
    if (use_cuda) {
        renderer->Render<RenderType::Shadow_GPU>(mesh, shadow_shader, depth_buffer);
    } else {
        renderer->Render<RenderType::Shadow_CPU>(mesh, shadow_shader, depth_buffer);
    }
    // std::cout << "----------- before phong render -----------" << std::endl;
    // for (auto &v: *mesh->GetVertexs()) {
    //     std::cout << v->pos << std::endl;
    // }
    if (use_cuda) {
        renderer->Render<RenderType::Phong_GPU>(mesh, phong_shader, zbuffer);
    } else {
        renderer->Render<RenderType::Phong_CPU>(mesh, phong_shader, zbuffer);
    }

    // std::cout << "after render" << std::endl;

    renderer->DrawImage(output_path);

    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end_time - start_time;

    std::cout << "FPS: " << 1.0f / duration.count()  << std::endl;

    // std::cout << "End!" << std::endl;

    return 0;
}

void CreateObject(Mesh &mesh, std::shared_ptr<std::vector<std::shared_ptr<Texture>>> texture_list) {
    auto material = MaterialLibrary::GetInstance().GetMaterial("silver");
    auto texture = texture_list->at(0);

    // Simple Object
    // mesh.AddVertex(vec3(0,1,0), 0.5, 0, vec3(0,0,1), Color::white, material, texture);
    // mesh.AddVertex(vec3(-1,0,0), 0, 0.5, vec3(0,0,1), Color::white, material, texture);
    // mesh.AddVertex(vec3(1,0,0), 1, 0.5, vec3(0,0,1), Color::white, material, texture);

    // simple cube
    // front
    mesh.AddVertex(vec3(-1,1,1), 0, 0, vec3(0,0,1), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,1), 1, 0, vec3(0,0,1), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,1), 0, 1, vec3(0,0,1), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,1), 1, 0, vec3(0,0,1), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,1), 0, 1, vec3(0,0,1), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,1), 1, 1, vec3(0,0,1), Color::white, material, texture);
    // back
    mesh.AddVertex(vec3(-1,1,-1), 0, 0, vec3(0,0,-1), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,-1), 1, 0, vec3(0,0,-1), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,-1), 0, 1, vec3(0,0,-1), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,-1), 1, 0, vec3(0,0,-1), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,-1), 0, 1, vec3(0,0,-1), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,-1), 1, 1, vec3(0,0,-1), Color::white, material, texture);
    // left
    mesh.AddVertex(vec3(-1,1,-1), 0, 0, vec3(-1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,1,1), 1, 0, vec3(-1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,-1), 0, 1, vec3(-1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,1,1), 1, 0, vec3(-1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,-1), 0, 1, vec3(-1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,1), 1, 1, vec3(-1,0,0), Color::white, material, texture);
    // right
    mesh.AddVertex(vec3(1,1,-1), 0, 0, vec3(1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,1), 1, 0, vec3(1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,-1), 0, 1, vec3(1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,1), 1, 0, vec3(1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,-1), 0, 1, vec3(1,0,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,1), 1, 1, vec3(1,0,0), Color::white, material, texture);
    // top
    mesh.AddVertex(vec3(-1,1,-1), 0, 0, vec3(0,1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,-1), 1, 0, vec3(0,1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,1,1), 0, 1, vec3(0,1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,-1), 1, 0, vec3(0,1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,1,1), 0, 1, vec3(0,1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,1,1), 1, 1, vec3(0,1,0), Color::white, material, texture);
    // bottom
    mesh.AddVertex(vec3(-1,-1,-1), 0, 0, vec3(0,-1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,-1), 1, 0, vec3(0,-1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,1), 0, 1, vec3(0,-1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,-1), 1, 0, vec3(0,-1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(-1,-1,1), 0, 1, vec3(0,-1,0), Color::white, material, texture);
    mesh.AddVertex(vec3(1,-1,1), 1, 1, vec3(0,-1,0), Color::white, material, texture);
}