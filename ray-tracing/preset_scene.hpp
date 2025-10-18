#pragma once
#include "hittable_list.hpp"
#include "bvh.hpp"
#include "material.hpp"
#include "sphere.hpp"

namespace lxrt {

class PresetScene {
public:
    static HittableList& RTSimpleScene() {
        static HittableList scene;
        if (scene.objects.size() != 0)
            return scene;

        auto material_ground = std::make_shared<Lambertian>(color(0.8, 0.8, 0.0));
        auto material_center = std::make_shared<Lambertian>(color(0.1, 0.2, 0.5));
        auto material_left   = std::make_shared<Dielectric>(1.5);
        auto material_bubble = std::make_shared<Dielectric>(1 / 1.5);
        auto material_right  = std::make_shared<Metal>(color(0.8, 0.6, 0.2), 1.0);

        scene.add(std::make_shared<Sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
        scene.add(std::make_shared<Sphere>(point3( 0.0,    0.0, -1.2),   0.5, material_center));
        scene.add(std::make_shared<Sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
        scene.add(std::make_shared<Sphere>(point3(-1.0,    0.0, -1.0),   0.4, material_bubble));
        scene.add(std::make_shared<Sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

        scene = HittableList(std::make_shared<BVHNode>(scene));  // construct BVH tree
        return scene;
    }

    static HittableList& RTTotalScene() {
        static HittableList scene;
        if (scene.objects.size() != 0)
            return scene;

        auto ground_material = std::make_shared<Lambertian>(color(0.5, 0.5, 0.5));
        scene.add(std::make_shared<Sphere>(point3(0,-1000,0), 1000, ground_material));

        for (int a = -11; a < 11; a++) {
            for (int b = -11; b < 11; b++) {
                auto choose_mat = random_double();
                point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

                if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                    std::shared_ptr<Material> sphere_material;

                    if (choose_mat < 0.8) {
                        // diffuse
                        auto albedo = color::random() * color::random();
                        sphere_material = std::make_shared<Lambertian>(albedo);
                        scene.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    } else if (choose_mat < 0.95) {
                        // metal
                        auto albedo = color::random(0.5, 1);
                        auto fuzz = random_double(0, 0.5);
                        sphere_material = std::make_shared<Metal>(albedo, fuzz);
                        scene.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    } else {
                        // glass
                        sphere_material = std::make_shared<Dielectric>(1.5);
                        scene.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    }
                }
            }
        }
        
        auto material1 = std::make_shared<Dielectric>(1.5);
        scene.add(std::make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));

        auto material2 = std::make_shared<Lambertian>(color(0.4, 0.2, 0.1));
        scene.add(std::make_shared<Sphere>(point3(-4, 1, 0), 1.0, material2));

        auto material3 = std::make_shared<Metal>(color(0.7, 0.6, 0.5), 0.0);
        scene.add(std::make_shared<Sphere>(point3(4, 1, 0), 1.0, material3));

        auto temp = std::make_shared<BVHNode>(scene);
        scene = HittableList(temp);  // construct BVH tree
        return scene;
    }
};

} // namespace lxrt