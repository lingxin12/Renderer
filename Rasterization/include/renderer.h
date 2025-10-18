#pragma once
#include <memory>
#include <vector>
#include <ranges>
#include <format>
#include <chrono>
#include "math_fun.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "buffer.h"

namespace lxrr {

enum class RenderType {
    Phong_CPU,
    Phong_GPU,
    Shadow_CPU,
    Shadow_GPU
};

class Renderer
{
public:
    Renderer(unsigned int sw, unsigned int sh,
             std::shared_ptr<Camera> camera,
             std::shared_ptr<std::vector<std::shared_ptr<Texture>>> texture_list,
             std::shared_ptr<std::vector<std::shared_ptr<DirectionLight>>> direction_light_list,
             std::shared_ptr<std::vector<std::shared_ptr<PointLight>>> point_light_list,
             mat4x4 light_V, mat4x4 light_P,
             std::shared_ptr<Shader> shader):
             screen_width(sw), screen_height(sh),
             camera(camera),
             texture_list(texture_list),
             direction_light_list(direction_light_list),
             point_light_list(point_light_list),
             light_V(light_V), light_P(light_P),
             shader(shader) {}

    ~Renderer() = default;

    template<RenderType render_type>
    bool Render(std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader, std::shared_ptr<DepthBuffer> zbuffer);

    std::shared_ptr<std::vector<std::shared_ptr<vec2>>> GetPixelCoordList() const { return pixel_coord_list; }
    std::shared_ptr<std::vector<std::shared_ptr<color4>>> GetColorList() const { return color_list; }

    bool DrawImage(const char* path);
                    

private:
    unsigned int screen_width, screen_height;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<std::vector<std::shared_ptr<Texture>>> texture_list;
    std::shared_ptr<std::vector<std::shared_ptr<DirectionLight>>> direction_light_list;
    std::shared_ptr<std::vector<std::shared_ptr<PointLight>>> point_light_list;

    mat4x4 light_V, light_P;

    std::shared_ptr<Shader> shader;

    std::shared_ptr<std::vector<std::shared_ptr<vec2>>> pixel_coord_list;
    std::shared_ptr<std::vector<std::shared_ptr<color4>>> color_list;
};

} // namespace lxrr