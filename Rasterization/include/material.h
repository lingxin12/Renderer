#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <string>
#include "vector.h"

namespace lxrr {

class PhongMaterial {
public:
    PhongMaterial():ambient(vec3(0.05,0.05,0.05)),diffuse(vec3(0.5,0.5,0.5)),specular(vec3(0.7,0.7,0.7)),shininess(0.078125) {
        SetUniqueID();
    }
    PhongMaterial(const vec3 &a, const vec3 &d, const vec3 &s, float sh):
        ambient(a), diffuse(d), specular(s), shininess(sh) {
        SetUniqueID();
    }

    std::string name;
    unsigned int id;
    inline static unsigned int current_count = 0;
    void SetUniqueID(std::string n = "None") { name = n; id = current_count++; }

public:
    ~PhongMaterial() = default;
    friend class MaterialLibrary;

    vec3 ambient, diffuse, specular;
    float shininess;
};



// Materials Table: http://devernay.free.fr/cours/opengl/materials.html
// Default Material: white rubber
class MaterialLibrary {
public:
    static MaterialLibrary &GetInstance() {
        static MaterialLibrary instance;
        return instance;
    }

    std::shared_ptr<PhongMaterial> GetMaterial(const std::string &name) {
        if (materials.find(name) == materials.end())
            return materials["default"];
        return materials[name];
    }

    void AddMaterial(const std::string &name, vec3 ambient, vec3 diffuse, vec3 specular, float shininess) {
        materials[name] = std::make_shared<PhongMaterial>(ambient, diffuse, specular, shininess);
    }

private:
    MaterialLibrary() {
        materials["default"] = std::make_shared<PhongMaterial>();
        materials["emerald"] = std::make_shared<PhongMaterial>(vec3(0.0215,0.1745,0.0215), vec3(0.07568,0.61424,0.07568), vec3(0.633,0.727811,0.633), 0.6);
        materials["jade"] = std::make_shared<PhongMaterial>(vec3(0.135,0.2225,0.1575), vec3(0.54,0.89,0.63), vec3(0.316228,0.316228,0.316228), 0.1);
        materials["obsidian"] = std::make_shared<PhongMaterial>(vec3(0.05375,0.05,0.06625), vec3(0.18275,0.17,0.22525), vec3(0.332741,0.328634,0.346435), 0.3);
        materials["pearl"] = std::make_shared<PhongMaterial>(vec3(0.25,0.20725,0.20725), vec3(1,0.829,0.829), vec3(0.296648,0.296648,0.296648), 0.088);
        materials["ruby"] = std::make_shared<PhongMaterial>(vec3(0.1745,0.01175,0.01175), vec3(0.61424,0.04136,0.04136), vec3(0.727811,0.626959,0.626959), 0.6);
        materials["turquoise"] = std::make_shared<PhongMaterial>(vec3(0.1,0.18725,0.1745), vec3(0.396,0.74151,0.69102), vec3(0.297254,0.30829,0.306678), 0.1);
        materials["brass"] = std::make_shared<PhongMaterial>(vec3(0.329412,0.223529,0.027451), vec3(0.780392,0.568627,0.113725), vec3(0.992157,0.941176,0.807843), 0.21794872);
        materials["bronze"] = std::make_shared<PhongMaterial>(vec3(0.2125,0.1275,0.054), vec3(0.714,0.4284,0.18144), vec3(0.393548,0.271906,0.166721), 0.2);
        materials["chrome"] = std::make_shared<PhongMaterial>(vec3(0.25,0.25,0.25), vec3(0.4,0.4,0.4), vec3(0.774597,0.774597,0.774597), 0.6);
        materials["copper"] = std::make_shared<PhongMaterial>(vec3(0.19125,0.0735,0.0225), vec3(0.7038,0.27048,0.0828), vec3(0.256777,0.137622,0.086014), 0.1);
        materials["gold"] = std::make_shared<PhongMaterial>(vec3(0.24725,0.1995,0.0745), vec3(0.75164,0.60648,0.22648), vec3(0.628281,0.555802,0.366065), 0.4);
        materials["silver"] = std::make_shared<PhongMaterial>(vec3(0.19225,0.19225,0.19225), vec3(0.50754,0.50754,0.50754), vec3(0.508273,0.508273,0.508273), 0.4);
        materials["black plastic"] = std::make_shared<PhongMaterial>(vec3(0.0,0.0,0.0), vec3(0.01,0.01,0.01), vec3(0.50,0.50,0.50), .25);
        materials["cyan plastic"] = std::make_shared<PhongMaterial>(vec3(0.0,0.1,0.06), vec3(0.0,0.50980392,0.50980392), vec3(0.50196078,0.50196078,0.50196078), .25);
        materials["green plastic"] = std::make_shared<PhongMaterial>(vec3(0.0,0.0,0.0), vec3(0.1,0.35,0.1), vec3(0.45,0.55,0.45), .25);
        materials["red plastic"] = std::make_shared<PhongMaterial>(vec3(0.0,0.0,0.0), vec3(0.5,0.0,0.0), vec3(0.7,0.6,0.6), .25);
        materials["white plastic"] = std::make_shared<PhongMaterial>(vec3(0.0,0.0,0.0), vec3(0.55,0.55,0.55), vec3(0.70,0.70,0.70), .25);
        materials["yellow plastic"] = std::make_shared<PhongMaterial>(vec3(0.0,0.0,0.0), vec3(0.5,0.5,0.0), vec3(0.60,0.60,0.50), .25);
        materials["black rubber"] = std::make_shared<PhongMaterial>(vec3(0.02,0.02,0.02), vec3(0.01,0.01,0.01), vec3(0.4,0.4,0.4), .078125);
        materials["cyan rubber"] = std::make_shared<PhongMaterial>(vec3(0.0,0.05,0.05), vec3(0.4,0.5,0.5), vec3(0.04,0.7,0.7), .078125);
        materials["green rubber"] = std::make_shared<PhongMaterial>(vec3(0.0,0.05,0.0), vec3(0.4,0.5,0.4), vec3(0.04,0.7,0.04), .078125);
        materials["red rubber"] = std::make_shared<PhongMaterial>(vec3(0.05,0.0,0.0), vec3(0.5,0.4,0.4), vec3(0.7,0.04,0.04), .078125);
        materials["white rubber"] = std::make_shared<PhongMaterial>(vec3(0.05,0.05,0.05), vec3(0.5,0.5,0.5), vec3(0.7,0.7,0.7), .078125);
        materials["yellow rubber"] = std::make_shared<PhongMaterial>(vec3(0.05,0.05,0.0), vec3(0.5,0.5,0.4), vec3(0.7,0.7,0.04), .078125);
    }

    std::unordered_map<std::string, std::shared_ptr<PhongMaterial>> materials;
};

}

