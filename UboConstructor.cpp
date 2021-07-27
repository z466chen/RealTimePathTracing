#include "UboConstructor.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "cs488-framework/GlErrorCheck.hpp"
#include <glm/ext.hpp>
#include <iostream>

std::vector<UboObject> UboConstructor::obj_arr = {};
std::vector<UboVertex> UboConstructor::vert_arr = {};
std::vector<UboElement> UboConstructor::elem_arr = {};
std::vector<UboMaterial> UboConstructor::mat_arr = {};
std::vector<UboBVH> UboConstructor::bvh_arr = {};
std::vector<UboBVH> UboConstructor::bvh_mesh_arr = {};
std::vector<UboPerlinNoise> UboConstructor::perlin_arr = {};
std::vector<UboLight> UboConstructor::light_arr = {};

static void __constructTexture(GLuint &id, void *arr, int mem_size, int texture_size) {
    float temp[texture_size*texture_size];
    memcpy(temp,arr,mem_size);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, texture_size, 
        texture_size, 0, GL_RED,  GL_FLOAT, (void *)temp);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void UboConstructor::construct(
    GLuint &m_ubo_obj, 
    GLuint &m_ubo_vert,
    GLuint &m_ubo_elem,
    GLuint &m_ubo_perlin,
    GLuint &m_ubo_bvh_tex,
    GLuint &m_ubo_bvh_mesh_tex,
    GLuint &m_ubo_mat,
    GLuint &m_ubo_bvh,
    GLuint &m_ubo_bvh_mesh,
    GLuint &m_ubo_light
) { 
    
    {
        const int texture_size = 1024;
        glActiveTexture(GL_TEXTURE0);
        
        // for (auto &obj: obj_arr) {
        //     obj.obj_type = 1.0f;
        // }

        __constructTexture(m_ubo_obj, (void*)&obj_arr[0], 
            184*obj_arr.size(), texture_size);
        CHECK_GL_ERRORS;
    }

    {
        const int texture_size = 512;

        glActiveTexture(GL_TEXTURE1);
        __constructTexture(m_ubo_vert, (void*)&vert_arr[0], 
            12*vert_arr.size(), texture_size);
        CHECK_GL_ERRORS;
    }

    {
        const int texture_size = 512;

        glActiveTexture(GL_TEXTURE2);
        __constructTexture(m_ubo_elem, (void*)&elem_arr[0], 
            12*elem_arr.size(), texture_size);
        CHECK_GL_ERRORS;
    }

    // {
    //     const int texture_size = 128;

    //     glActiveTexture(GL_TEXTURE3);
    //     __constructTexture(m_ubo_perlin, (void*)&perlin_arr[0], 
    //         16*perlin_arr.size(), texture_size);
    //     CHECK_GL_ERRORS;
    // }

    {
        if (bvh_arr.size() > 1024) {
            const int texture_size = 512;

            glActiveTexture(GL_TEXTURE4);
            __constructTexture(m_ubo_bvh_tex, (void*)&bvh_arr[1024], 
                48*(bvh_arr.size() - 1024), texture_size);
            CHECK_GL_ERRORS;
        }
    }

    {
        if (bvh_mesh_arr.size() > 1024) {
            const int texture_size = 1024;

            glActiveTexture(GL_TEXTURE5);
            __constructTexture(m_ubo_bvh_mesh_tex, (void*)&bvh_mesh_arr[1024], 
                48*(bvh_mesh_arr.size() - 1024), texture_size);
            CHECK_GL_ERRORS;
        }
    }

    {
        glGenBuffers(1, &m_ubo_perlin);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_perlin);
        glBufferData(GL_UNIFORM_BUFFER, 16*514, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_ubo_perlin, 0, 16*514);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_perlin);
        for (int i = 0; i < perlin_arr.size(); ++i) {
            auto &obj = perlin_arr[i];
            glBufferSubData(GL_UNIFORM_BUFFER, i*16+0, 12, glm::value_ptr(obj.g));
            glBufferSubData(GL_UNIFORM_BUFFER, i*16+12, 4, &obj.p);
       }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    {
        glGenBuffers(1, &m_ubo_mat);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_mat);
        glBufferData(GL_UNIFORM_BUFFER, 48*100, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_ubo_mat, 0, 48*100);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_mat);
        for (int i = 0; i < mat_arr.size(); ++i) {
            auto &obj = mat_arr[i];
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+0, 8, glm::value_ptr(obj.mat_data_1));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+8, 8, glm::value_ptr(obj.mat_data_2));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+16, 8, glm::value_ptr(obj.mat_data_3));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+24, 8, glm::value_ptr(obj.mat_data_4));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+32, 4, &obj.mat_type);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    {
        glGenBuffers(1, &m_ubo_bvh);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh);
        glBufferData(GL_UNIFORM_BUFFER, 48*1024, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_ubo_bvh, 0, 48*1024);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh);

        int size = (bvh_arr.size() < 1024)? bvh_arr.size():1024;

        for (int i = 0; i < size; ++i) {
            auto &obj = bvh_arr[i];
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+0, 8, glm::value_ptr(obj.bvh_aabb_1));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+8, 8, glm::value_ptr(obj.bvh_aabb_2));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+16, 8, glm::value_ptr(obj.bvh_aabb_3));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+24, 4, &obj.bvh_left);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+28, 4, &obj.bvh_right);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+32, 4, &obj.obj_id_1);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+36, 4, &obj.obj_id_2);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+40, 4, &obj.obj_id_3);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+44, 4, &obj.obj_id_4);            
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        CHECK_GL_ERRORS;
    }


    {
        glGenBuffers(1, &m_ubo_bvh_mesh);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh_mesh);
        glBufferData(GL_UNIFORM_BUFFER, 48*1024, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 2, m_ubo_bvh_mesh, 0, 48*1024);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh_mesh);

        int size = (bvh_mesh_arr.size() < 1024)? bvh_mesh_arr.size():1024;

        for (int i = 0; i < size; ++i) {
            auto &obj = bvh_mesh_arr[i];
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+0, 8, glm::value_ptr(obj.bvh_aabb_1));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+8, 8, glm::value_ptr(obj.bvh_aabb_2));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+16, 8, glm::value_ptr(obj.bvh_aabb_3));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+24, 4, &obj.bvh_left);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+28, 4, &obj.bvh_right);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+32, 4, &obj.obj_id_1);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+36, 4, &obj.obj_id_2);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+40, 4, &obj.obj_id_3);
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+44, 4, &obj.obj_id_4);            
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    {
        glGenBuffers(1, &m_ubo_light);
        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_light);
        glBufferData(GL_UNIFORM_BUFFER, 48*1024, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 3, m_ubo_light, 0, 48*1024);

        glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_light);

        for (int i = 0; i < light_arr.size(); ++i) {
            auto &obj = light_arr[i];
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+0, 12, glm::value_ptr(obj.color));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+16, 12, glm::value_ptr(obj.position));
            glBufferSubData(GL_UNIFORM_BUFFER, i*48+32, 12, glm::value_ptr(obj.falloff));          
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        CHECK_GL_ERRORS;
    }
}

