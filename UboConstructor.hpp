#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "cs488-framework/OpenGLImport.hpp"

enum class UboPrimitiveType {
  SPHERE=0,
  BOX=1,
  ROUNDBOX=2,
  CYLINDER=3,
  TORUS=4,
  MESH=5,
  CSG=6
};

enum class UboMaterialType {
    PHONG_DIFFUSE=0,
    PHONG_SPECULAR,
    SOLID_DIFFUSE,
    SOLID_SPECULAR
};

struct UboObject{
    glm::mat4 t_matrix; // 0
    glm::mat4 inv_t_matrix; // 64
    glm::vec2 obj_aabb_1; // 128
    glm::vec2 obj_aabb_2; // 136
    glm::vec2 obj_aabb_3; // 144
    glm::vec2 obj_data_1; // 152
    glm::vec2 obj_data_2; // 160
    glm::vec2 obj_data_3; // 168
    float obj_type; // 176
    float mat_id; // 180
                // 184
};

struct UboVertex {
    glm::vec3 pos; // 0
                   // 12
};

struct UboElement {
    float v1; // 0
    float v2; // 4
    float v3; // 8
            // 12
};

struct UboMaterial {
    glm::vec2 mat_data_1; // 0
    glm::vec2 mat_data_2; // 8
    glm::vec2 mat_data_3; // 16
    glm::vec2 mat_data_4; // 24
    float mat_type; // 32
                // 36
};

struct UboBVH {
    glm::vec2 bvh_aabb_1; // 0
    glm::vec2 bvh_aabb_2; // 8
    glm::vec2 bvh_aabb_3; // 16
    float bvh_left; // 24
    float bvh_right; // 28
    float obj_id_1; // 32
    float obj_id_2; // 36
    float obj_id_3; // 40
    float obj_id_4; // 44
            // 48
};

struct UboPerlinNoise {
    glm::vec3 g; // 0
    float p; // 16
        // 20
};

struct UboLight {
    glm::vec3 color; // 0
    glm::vec3 position; // 12
    glm::vec3 falloff; // 24
                    // 36
};

class UboConstructor {
public:
    static std::vector<UboObject> obj_arr;
    static std::vector<UboVertex> vert_arr;
    static std::vector<UboElement> elem_arr;
    static std::vector<UboMaterial> mat_arr;
    static std::vector<UboBVH> bvh_arr;
    static std::vector<UboBVH> bvh_mesh_arr;
    static std::vector<UboPerlinNoise> perlin_arr;
    static std::vector<UboLight> light_arr;

    static void construct(
        GLuint &m_ubo_obj, 
        GLuint &m_ubo_vert,
        GLuint &m_ubo_elem,
        GLuint &m_ubo_bvh_tex,
        GLuint &m_ubo_bvh_mesh_tex,
        GLuint &m_ubo_perlin,
        GLuint &m_ubo_mat,
        GLuint &m_ubo_bvh, 
        GLuint &m_ubo_bvh_mesh,
        GLuint &m_ubo_light
    );
};