#pragma once
#include <string>
#include "memory"
#include "color.h"

namespace lxrr {

class Texture {
public:
    Texture() {
        width = 0;
        height = 0;
        texture_data = nullptr;

        SetUniqueID();
    }

    ~Texture() = default;

    void LoadTexture(const char* path);
    color4 Sample(float u, float v);
    float Clamp(float min, float max, float value);
    unsigned int GetWidth() const { return width; }
    unsigned int GetHeight() const { return height; }
    color4* GetTextureDataPtr() { return texture_data.get(); }

    std::string name;
    unsigned int id;
    inline static unsigned int current_count = 0;
    void SetUniqueID(std::string n = "None") { name = n; id = current_count++; }

private:
    unsigned int width, height;
    std::shared_ptr<color4[]> texture_data;
};

} // namespace lxrr