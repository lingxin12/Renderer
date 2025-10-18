#include "texture.h"
#include "stb/stb_image.h"

namespace lxrr {

void Texture::LoadTexture(const char* path) {
    int tex_width, tex_height, tex_channels;
    auto tex_image = stbi_load(path, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    if(!tex_image) {
        std::string errorMsg = "Failed to load texture image: " + std::string(path) + "\n"
                               "Error: " + stbi_failure_reason();
        throw std::runtime_error(errorMsg);
    }

    width = tex_width;
    height = tex_height;
    texture_data = std::make_shared<color4[]>(width * height);

    for (auto j = 0; j < height; ++ j) {
        for (auto i = 0; i < width; ++ i) {
            auto pixel = tex_image + (j * width * tex_channels + i * tex_channels);
            float r = static_cast<uint32_t>(pixel[0]) / 255.99f;
            float g = static_cast<uint32_t>(pixel[1]) / 255.99f;
            float b = static_cast<uint32_t>(pixel[2]) / 255.99f;
            // float a = static_cast<uint32_t>(pixel[3]) / 255.99f;
            texture_data[j * width + i] = color4(r, g, b);
        }
    }
}

color4 Texture::Sample(float u, float v) {
    u = Clamp(0, 0.999999f, u);
    v = Clamp(0, 0.999999f, v);
    int i = static_cast<int>(u * width);
    int j = static_cast<int>(v * height);
    return texture_data[j * width + i];
}

float Texture::Clamp(float min, float max, float value) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


} // namespace lxrr