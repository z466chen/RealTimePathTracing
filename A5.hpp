#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "Camera.hpp"
#include "Scene.hpp"

class A5: public CS488Window {
    
public:
    static CS488Window *window;

    A5(SceneNode *root, 
        glm::vec3 &eye, glm::vec3 &view, glm::vec3 &up, double fov, 
        glm::vec3 &ambient, std::list<Light *> &lights);
    virtual ~A5();
protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
    struct State {
        bool is_mouse_pos_init = false;
        glm::vec2 mouse_pos_prev = glm::vec2(0.0f);
        float prev_time_app = 0.0f;
        float prev_time_mouse = 0.0f;
    } state;

    Camera camera;
    glm::vec3 ambient;
    int bvh_id;

    float total_area;

    ShaderProgram m_rt_shader;

    GLuint m_vao_quad;
    GLuint m_vbo_quad;

    GLuint m_ubo_obj;
    GLuint m_ubo_vert;
    GLuint m_ubo_elem;
    GLuint m_ubo_bvh_tex;
    GLuint m_ubo_bvh_mesh_tex;
    GLuint m_ubo_perlin;
    GLuint m_ubo_mat;
    GLuint m_ubo_bvh;
    GLuint m_ubo_bvh_mesh;
    GLuint m_ubo_light;
    GLuint m_camera;

    GLuint uobj;
    GLuint uvert;
    GLuint uelem;
    GLuint umat;
    GLuint ubvh;
    GLuint ubvhMesh;
    GLuint uperlin;
    GLuint ulight;
    GLuint ubvhTex;
    GLuint ubvhMeshTex;
    GLuint ucamera;

    GLuint uambient;
    GLuint unol;
    GLuint uwsize;
    GLuint utotalArea;
    GLuint useed;

    GLuint m_bg_texture;
    GLuint ubackground;

    // initialization functions
    void init_shaders();
    void init_quad();
    void init_textures();
    void init_uniforms();
};