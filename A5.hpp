#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "Camera.hpp"
#include "Scene.hpp"
#include <unordered_map>

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
        // int pipeline_code;
        int frame_buffer_mode = 0;
        bool frame_initialized = false; 
    } state;

    Camera camera;
    glm::vec3 ambient;
    int bvh_id;

    float total_area;

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

    GLuint m_bg_texture;
    GLuint m_gbuffer;

    struct MeshBufferObject {
        GLuint m_vao_mesh_gb;
        GLuint m_vbo_pos_gb;
        GLuint m_vbo_normal_gb;
    };
    
    std::unordered_map<int,MeshBufferObject> m_mbo_gb;

    struct FrameBufferObject {
        GLuint fbo;
        GLuint data_objs[7];
        int number;
    };

    FrameBufferObject fbos[7];
    


    class ISBShaderObject {
    public:
        const A5 &parent;

        const char * vs_name = "shaders/Restir/InitialSample/Default.vs";
        const char * fs_name = "shaders/Restir/InitialSample/InitialSample.fs";

        ShaderProgram shader;

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
        GLuint uPrevPosTex;
        GLuint uPrevNormalTex;
        GLuint uPrevIdTex;

        GLuint uambient;
        GLuint unol;
        GLuint uwsize;
        GLuint utotalArea;
        GLuint useed;
        GLuint ubackground;

    
        ISBShaderObject(const A5 &x):parent{x} {}
        void bindObjects();
        void create();
    } isb_shader_object = ISBShaderObject(*this);

    class TRBShaderObject {
    public:
        const A5 &parent;

        const char * vs_name = "shaders/Restir/TemporalSample/Default.vs";
        const char * fs_name = "shaders/Restir/TemporalSample/TemporalSample.fs";

        ShaderProgram shader;

        GLuint uisb_tex1;
        GLuint uisb_tex2;
        GLuint uisb_tex3;
        GLuint uisb_tex4;
        GLuint uisb_tex5;
        GLuint uisb_tex6;
        
        GLuint utrb_tex1;
        GLuint utrb_tex2;
        GLuint utrb_tex3;
        GLuint utrb_tex4;
        GLuint utrb_tex5;
        GLuint utrb_tex6;
        GLuint utrb_tex7;

        GLuint useed;
        GLuint uinitialized;
        
        TRBShaderObject(const A5 &x):parent{x} {}
        void bindObjects();
        void create();
    } trb_shader_object = TRBShaderObject(*this);

    class SRBShaderObject {
    public:
        const A5 &parent;

        const char * vs_name = "shaders/Restir/SpatialSample/Default.vs";
        const char * fs_name = "shaders/Restir/SpatialSample/SpatialSample.fs";

        ShaderProgram shader;

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
        GLuint uPrevPosTex;
        GLuint uPrevNormalTex;
        GLuint uPrevIdTex;

        GLuint uambient;
        GLuint unol;
        GLuint uwsize;
        GLuint utotalArea;
        GLuint useed;
        GLuint ubackground;

        GLuint usrb_tex1;
        GLuint usrb_tex2;
        GLuint usrb_tex3;
        GLuint usrb_tex4;
        GLuint usrb_tex5;
        GLuint usrb_tex6;
        GLuint usrb_tex7;

        GLuint utrb_tex1;
        GLuint utrb_tex2;
        GLuint utrb_tex3;
        GLuint utrb_tex4;
        GLuint utrb_tex5;
        GLuint utrb_tex6;
        GLuint utrb_tex7;

    
        SRBShaderObject(const A5 &x):parent{x} {}
        void bindObjects();
        void create();
    } srb_shader_object = SRBShaderObject(*this);

    class IGShaderObject {
    public:
        const A5 &parent;

        const char * vs_name = "shaders/Restir/ImageGenerate/Default.vs";
        const char * fs_name = "shaders/Restir/ImageGenerate/ImageGenerate.fs";

        ShaderProgram shader;

        GLuint uemission_tex;
        GLuint ucolor_buffer;
        GLuint unumber_buffer;
        
        GLuint usrb_tex1;
        GLuint usrb_tex2;
        GLuint usrb_tex3;
        GLuint usrb_tex4;
        GLuint usrb_tex5;
        GLuint usrb_tex6;
        GLuint usrb_tex7;
        
        IGShaderObject(const A5 &x):parent{x} {}
        void bindObjects();
        void create();
    } ig_shader_object = IGShaderObject(*this);

    // initialization functions
    void init_shaders();
    void init_quad();
    // void init_mesh_gb();
    void init_textures();
    void init_uniforms();
    void init_fbo(FrameBufferObject &obj, int number);
    void use_fbo(FrameBufferObject &obj, GLuint TexOffset, int number);
    void attach_fbo(FrameBufferObject &obj);
    void camera_update(GLuint camera_id);
};