#pragma once
#include <iostream>
#include <cmath>
#include <memory>
#include "common.h"
#include "vec3.hpp"
#include "process_bar.hpp"
#include "image_controller.hpp"
#include "ray.hpp"
#include "hittable.h"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "bvh.hpp"
#include "preset_scene.hpp"

using namespace lxrt;

color ray_color(const Ray& r, int depth, const Hittable& scene) {
    if (depth <= 0)
        return color(0,0,0);

    HitRecord rec;

    if (scene.hit(r, 0.001, kinfinity, rec)) {
        Ray scattered;
        color attenuation;
        if (rec.material->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth-1, scene);
        return color(0,0,0);
    }

    auto unit_dir = r.direction();
    auto a = 0.5 * (unit_dir.y() + 1.0);
    return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
}

int main() {
    // Image

    // int image_width = 1920;
    // int image_height = 1080;
    int image_width = 300;
    int image_height = 300;
    double fov = 20;
    auto camera_center = vec3(13,2,3);
    auto look_at = vec3(0,0,0);
    Camera camera(image_width, image_height, fov, camera_center, look_at);

    // Scene

    // auto scene = PresetScene::RTSimpleScene();
    auto scene = PresetScene::RTTotalScene();

    // Render
    ProcessBar process_bar(image_height, "Rendering");
    int samples_per_pixel = 500;
    int max_depth = 50;

    for (int j = 0; j < camera.image_height(); ++j) {
        for (int i = 0; i < camera.image_width(); ++i) {
            color pixel_color(0,0,0);
            for (int sample = 0; sample < samples_per_pixel; ++sample) {
                Ray r = camera.get_ray(i, j);
                pixel_color += ray_color(r, max_depth, scene);
            }
            camera.write_color(pixel_color / samples_per_pixel, j, i);
        }
        process_bar.update(j+1);
    }

    camera.save_image("output.png");

    return 0;
}