#pragma once
#include "vec3.hpp"
#include "ray.hpp"
#include "image_controller.hpp"

#include "cuda.h"
#include "cuda_runtime.h"
#include "vector_types.h"

#include "auxiliary_cuda.h"

namespace lxrt {

class Camera {
public:
    Camera(): image_width_(100), image_height_(100), fov_(90), look_at_(vec3(0,0,0)), look_up_(vec3(0,1,0)), image_(ImageController(image_height_, image_width_, 3)) {
    }

    Camera(int image_width, int image_height, double fov,
           const vec3& camera_center=vec3(0,0,0), const vec3& look_at=vec3(0,0,-1),
           const vec3& look_up=vec3(0,1,0), std::string image_mode="rgb")
        : image_width_(image_width), image_height_(image_height), fov_(fov), 
          camera_center_(camera_center), look_at_(look_at), look_up_(look_up)
    {
        auto aspect = (double)image_width / image_height;

        auto focal_length = (camera_center_ - look_at_).length();
        auto tan_fov = std::tan(degrees_to_radians(fov) / 2);
        auto viewport_height = 2 * tan_fov * focal_length;
        auto viewport_width = viewport_height * aspect;

        w = unit_vector(camera_center_ - look_at_);
        u = unit_vector(cross(look_up_, w));
        v = unit_vector(cross(w, u));

        auto viewport_u = viewport_width * u;
        auto viewport_v = viewport_height * -v;
        pixel_delta_u_ = viewport_u / image_width;
        pixel_delta_v_ = viewport_v / image_height;

        auto viewport_upper_left = camera_center_ - focal_length * w - viewport_u/2 - viewport_v/2;
        pixel_00_local_ = viewport_upper_left + 0.5 * (pixel_delta_u_ + pixel_delta_v_);

        if (image_mode == "rgb")
            image_ = ImageController(image_height, image_width, 3);
        else if (image_mode == "rgba")
            image_ = ImageController(image_height, image_width, 4);
        else
            throw std::invalid_argument("image_mode must be rgb or rgba");
    }

    int image_width() const { return image_width_; }
    int image_height() const { return image_height_; }
    point3& camera_center() { return camera_center_; }
    point3 get_camera_center() const { return camera_center_; }
    point3 get_pixel_00_local() const { return pixel_00_local_; }
    vec3 get_pixel_delta_u() const { return pixel_delta_u_; }
    vec3 get_pixel_delta_v() const { return pixel_delta_v_; }

    Ray get_ray(int i, int j, bool random_sample=true) {
        auto offset = vec3::random(-0.5, 0.5);

        auto pixel_center = pixel_00_local_ + ((i + offset.x()) * pixel_delta_u_) + ((j + offset.y()) * pixel_delta_v_);
        auto ray_direction = unit_vector(pixel_center - camera_center_);
        return Ray(camera_center_, ray_direction);
    }

    void write_color(color pixel_color, int j, int i) {
        image_.write(pixel_color, j, i);
    }

    void save_image(std::string filename) {
        image_.save(filename.c_str());
    }


private:
    int image_width_;
    int image_height_;
    point3 camera_center_;
    vec3 look_at_;
    vec3 look_up_;
    vec3 pixel_delta_u_;
    vec3 pixel_delta_v_;
    point3 pixel_00_local_;
    ImageController image_;
    double fov_;
    vec3 u, v, w;
};

} // namespace lxrt