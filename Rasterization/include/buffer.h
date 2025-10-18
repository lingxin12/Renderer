#pragma once
#include <memory>
#include "math_fun.h"

namespace lxrr {

class Buffer
{
public:
    Buffer() = default;
    virtual ~Buffer() = default;

    virtual float Get(int i, int j) const = 0;
    virtual void Write(int i, int j, float v) = 0;
};

class DepthBuffer: public Buffer
{
public:
    DepthBuffer(unsigned int width, unsigned int height):width(width), height(height) {
        depth_buffer = std::make_unique<float[]>(width * height);
        for (int i = 0; i < width * height; i++) {
            depth_buffer[i] = 1.0f;
        }
    }
    ~DepthBuffer() = default;

    // Get(H, W)
    float Get(int i, int j) const override {
        int y = clamp(0, height - 1, i);
        int x = clamp(0, width - 1, j);
        return depth_buffer[y * width + x];
    }

    void Write(int i, int j, float v) override {
        int y = clamp(0, height - 1, i);
        int x = clamp(0, width - 1, j);
        depth_buffer[y * width + x] = v;
    }

    unsigned int GetWidth() const {
        return width;
    }

    unsigned int GetHeight() const {
        return height;
    }

private:
    unsigned int width, height;
    std::unique_ptr<float[]> depth_buffer;
};

} //namespace lxrr