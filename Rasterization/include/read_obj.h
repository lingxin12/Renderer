#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tuple>
#include <cctype>
#include "vertex.h"
#include "texture.h"
#include "material.h"
#include "mesh.h"

namespace lxrr {

struct FaceData {
    vec3i v_id, vn_id, vt_id;
    std::string g_name;
    std::string mtl_name;
    std::string s;
};

struct OBJData {
    std::string mtl_name;
    std::vector<vec3> v_list;
    std::vector<vec3> vn_list;
    std::vector<vec2> vt_list;
    std::vector<FaceData> f_list;
};

class ReadOBJ {
public:
    static std::shared_ptr<Mesh> Read(
        const std::string &obj_path,
        const std::vector<std::string> &texture_path_list = std::vector<std::string>(),  // invalid
        const std::string &material_path = ""); // invalid

    static FaceData ProcessFace(const std::string &line, int flag);

    static std::tuple<int, int, int> ReadFaceVertex(const std::string &s, const std::string &divider);

    inline static std::shared_ptr<Texture> default_texture = nullptr;
    inline static std::shared_ptr<PhongMaterial> default_material = MaterialLibrary::GetInstance().GetMaterial("silver");

private:
    ReadOBJ() = default;
    ~ReadOBJ() = default;
};

} // namespace lxrr
