#include "lighting_model.h"

namespace lxrr {

color4 BlinnPhongModel(const vec3 &pos, const vec3 &normal, const vec3 &view_pos,
                       const DirectionLight &light, const PhongMaterial &material) {
    vec3 diffuse(0,0,0), specular(0,0,0), ambient(0.05,0.05,0.05);
    float shininess = material.shininess;
    float kd = light.get_kd(), ks = light.get_ks(), ka = light.get_ka();
    float attenuation = 1.0f / std::pow((pos - light.get_pos()).length(), 2);

    vec3 light_dir = light.get_dir().normalized();
    diffuse = diffuse + std::max(0.0f, vec3::dot(light_dir, normal)) * light.get_intensity() * light.get_color().get_3d();

    auto h = ((view_pos - pos).normalized() + light_dir).normalized();
    specular = specular + std::pow(std::max(0.0f, vec3::dot(normal, h)), shininess) * light.get_intensity() * light.get_color().get_3d();

    color4 result = color4(ka * ambient + (kd * diffuse + ks * specular) * attenuation);

    return result;
}


color4 BlinnPhongModel(const vec3 &pos, const vec3 &normal, const vec3 &view_pos,
                       const PointLight &light, const PhongMaterial &material) {
    return color4();
}

} // namespace lxrr