#include "read_obj.h"

namespace lxrr {

std::shared_ptr<Mesh> ReadOBJ::Read(
    const std::string &obj_path,
    const std::vector<std::string> &texture_path_list,  // invalid
    const std::string &material_path) {  // invalid

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    OBJData obj_data;

    std::ifstream obj_file(obj_path);
    std::string line;

    std::string current_g_name = "";
    std::string current_mtl_name = "";
    std::string current_s = "";

    if (obj_file.is_open()) {
        while (getline(obj_file, line)) {
            if (line.substr(0,6) == "mtllib") {
                obj_data.mtl_name = line.substr(7);
            } else if (line.substr(0,2) == "v ") {
                std::istringstream iss(line);
                std::string pre_s;
                float x, y, z;
                iss >> pre_s >> x >> y >> z;
                obj_data.v_list.push_back(vec3(x,y,z));
            } else if (line.substr(0,2) == "vn") {
                std::istringstream iss(line);
                std::string pre_s;
                float x, y, z;
                iss >> pre_s >> x >> y >> z;
                obj_data.vn_list.push_back(vec3(x,y,z));
            } else if (line.substr(0,2) == "vt") {
                std::istringstream iss(line);
                std::string pre_s;
                float u, v;
                iss >> pre_s >> u >> v;
                obj_data.vt_list.push_back(vec2(u,v));
            } else if (line.substr(0,2) == "f ") {
                obj_data.f_list.push_back(ProcessFace(line, 0));
                obj_data.f_list[obj_data.f_list.size()-1].g_name = current_g_name;
                obj_data.f_list[obj_data.f_list.size()-1].mtl_name = current_mtl_name;
                obj_data.f_list[obj_data.f_list.size()-1].s = current_s;

                obj_data.f_list.push_back(ProcessFace(line, 1));
                obj_data.f_list[obj_data.f_list.size()-1].g_name = current_g_name;
                obj_data.f_list[obj_data.f_list.size()-1].mtl_name = current_mtl_name;
                obj_data.f_list[obj_data.f_list.size()-1].s = current_s;
            } else if (line.substr(0,2) == "g ") {
                std::istringstream iss(line);
                std::string pre_s;
                iss >> pre_s >> current_g_name;
            } else if (line.substr(0,2) == "s ") {
                std::istringstream iss(line);
                std::string pre_s;
                iss >> pre_s >> current_s;
            } else if (line.substr(0,2) == "usemtl") {
                std::istringstream iss(line);
                std::string pre_s;
                iss >> pre_s >> current_mtl_name;
            }
        }
    } else {
        throw std::runtime_error("Can't open file: " + obj_path);
    }

    bool exist_texture = false;

    for (auto &f_data: obj_data.f_list) {
        for (int i = 0; i < 3; i ++) {
            if (exist_texture) {
                // std::cout << " test add 111 : " << obj_data.v_list[f_data.v_id[i]-1] << " " << obj_data.vn_list[f_data.vn_id[i]-1] << " " << default_material << " " << default_texture << std::endl;
                mesh->AddVertex(
                    obj_data.v_list[f_data.v_id[i]-1],
                    obj_data.vt_list[f_data.vt_id[i]-1].x(),
                    obj_data.vt_list[f_data.vt_id[i]-1].y(),
                    obj_data.vn_list[f_data.vn_id[i]-1],
                    Color::white,
                    default_material,
                    default_texture
                );
            } else {
                // std::cout << " test add 222 : " << obj_data.v_list[f_data.v_id[i]-1] << " " << obj_data.vn_list[f_data.vn_id[i]-1] << " " << default_material << " " << default_texture << std::endl;
                // std::cout << " test add 222 : " << f_data.v_id[i]-1 << " " << obj_data.v_list.size() << " " << f_data.vn_id[i]-1 << " " << obj_data.vn_list.size() << std::endl;
                mesh->AddVertex(
                    obj_data.v_list[f_data.v_id[i]-1],
                    0.0f, 0.0f,
                    obj_data.vn_list[f_data.vn_id[i]-1],
                    Color::white,
                    default_material,
                    default_texture
                );
            }
        }
    }

    return mesh;
}

FaceData ReadOBJ::ProcessFace(const std::string &line, int flag) {
    FaceData f_data;
    std::istringstream iss(line);
    std::string pre_s;
    std::string t_v[4];

    iss >> pre_s >> t_v[0] >> t_v[1] >> t_v[2] >> t_v[3];

    // only v
    if (t_v[0].find('/') == std::string::npos) {
        // ......
        std::cout << "Only v dont support" << std::endl;
        exit(0);
    } else if (t_v[0].find('//') != std::string::npos) {
        if (flag == 0) {
            std::tie(f_data.v_id[0], f_data.vt_id[0], f_data.vn_id[0]) = ReadFaceVertex(t_v[0], "//");
            std::tie(f_data.v_id[1], f_data.vt_id[1], f_data.vn_id[1]) = ReadFaceVertex(t_v[1], "//");
            std::tie(f_data.v_id[2], f_data.vt_id[2], f_data.vn_id[2]) = ReadFaceVertex(t_v[2], "//");
        }
        if (flag == 1) {
            std::tie(f_data.v_id[0], f_data.vt_id[0], f_data.vn_id[0]) = ReadFaceVertex(t_v[0], "//");
            std::tie(f_data.v_id[1], f_data.vt_id[1], f_data.vn_id[1]) = ReadFaceVertex(t_v[2], "//");
            std::tie(f_data.v_id[2], f_data.vt_id[2], f_data.vn_id[2]) = ReadFaceVertex(t_v[3], "//");
        }
    } else if (t_v[0].find('/') != std::string::npos) {
        // ......
        std::cout << "/ dont support" << std::endl;
        exit(0);
    }

    return f_data;
}

std::tuple<int, int, int> ReadOBJ::ReadFaceVertex(const std::string &s, const std::string &divider) {
    int v_id, vn_id, vt_id;

    size_t start_p = 0;
    size_t end_p = s.find(divider);
    std::string sub_s = s.substr(start_p, end_p - start_p);
    size_t s_p = sub_s.find_first_not_of(" \t"), e_p = sub_s.find_last_not_of(" \t");
    v_id = std::stoi(sub_s.substr(s_p, e_p - s_p + 1));
    
    start_p = end_p + divider.size();
    if (s.find(divider, start_p) == std::string::npos) {
        vt_id = -1;
        std::string sub_s = s.substr(start_p);
        s_p = sub_s.find_first_not_of(" \t"), e_p = sub_s.find_last_not_of(" \t");
        vn_id = std::stoi(sub_s.substr(s_p, e_p - s_p + 1));
    } else {
        end_p = s.find(divider, start_p);
        std::string sub_s = s.substr(start_p, end_p - start_p);
        s_p = sub_s.find_first_not_of(" \t"), e_p = sub_s.find_last_not_of(" \t");
        vt_id = std::stoi(sub_s.substr(s_p, e_p - s_p + 1)); ///

        start_p = end_p + divider.size();
        sub_s = s.substr(start_p);
        s_p = sub_s.find_first_not_of(" \t"), e_p = sub_s.find_last_not_of(" \t");
        vn_id = std::stoi(sub_s.substr(s_p, e_p - s_p + 1)); ///
    }
    return std::make_tuple(v_id, vt_id, vn_id);
}

} // namespace lxrr