#include "A5.hpp"
#include <imgui/imgui.h>
#include <lodepng/lodepng.h>
#include <iostream>
#include <glm/ext.hpp>
#include "UboConstructor.hpp"
#include "BVH.hpp"
#include "general.hpp"
#include <fstream>

#include "cs488-framework/GlErrorCheck.hpp"

#define SPP 1

const float CAMERA_TRANSLATION_SPEED = 200.0f;
const float CAMERA_ROTATE_SPEED = 9.0f;


CS488Window * A5::window = nullptr;

static int __loadTextureRGBA(GLuint &textureID, const char *filePath) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	unsigned int height, width;
	std::vector<unsigned char> data;
	
	unsigned int error = lodepng::decode(data, height, width, filePath);
	if (error != 0) {
		std::cout << "ERROR::LOAD_TEXTURE_ERROR::" << lodepng_error_text(error) << std::endl;
		return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 
		0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	return 0;
}

A5::A5(SceneNode *root, 
		glm::vec3 &eye, glm::vec3 &view, glm::vec3 &up, double fov, 
		glm::vec3 &ambient, std::list<Light *> &lights):
	camera{(float)fov, eye,glm::normalize(view - eye),  glm::normalize(up)},
	ambient{ambient} {
	BVH bvh{root};
	glm::mat4 unit = glm::mat4(1.0f);
	bvh_id = bvh.construct(unit);

	total_area = 0.0f;
	for (auto &l: UboConstructor::light_arr) {
		total_area += l.oid_and_area.y;
	}
	
	for (auto l: lights) {
		l->construct(unit);
	}
}

void A5::init_shaders() {
	UboConstructor::calcDefines();
	m_rt_shader.generateProgramObject();

	std::string frag_shader_src;
	frag_shader_src = readFile(getAssetFilePath("shaders/PathTrace.fs"));
	
	replace(frag_shader_src, "{OBJ_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.obj_texture_size));
	replace(frag_shader_src, "{VERT_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.vert_texture_size));
	replace(frag_shader_src, "{ELEM_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.elem_texture_size));
	replace(frag_shader_src, "{BVH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_texture_size));
	replace(frag_shader_src, "{BVH_MESH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_mesh_texture_size));

	std::ofstream ofs;
	ofs.open (getAssetFilePath("shaders/temp.fs"), std::ofstream::out);
	ofs << frag_shader_src;
	ofs.close();

	m_rt_shader.attachVertexShader(
		getAssetFilePath("shaders/Default.vs").c_str()
	);
    m_rt_shader.attachFragmentShader(
        getAssetFilePath("shaders/temp.fs").c_str()
    );

    m_rt_shader.link();
	CHECK_GL_ERRORS;
}

void A5::init_quad() {
	float verts[] = 
	{
		-1.0f, -1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 0.0f,

		-1.0f, -1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 0.0f
	};

	// Create VAO
	glGenVertexArrays(1, &m_vao_quad);
	glBindVertexArray(m_vao_quad);

	// Create VBO
	glGenBuffers(1, &m_vbo_quad);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	// Attach Vertex Attributes
	glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, 
        GL_FALSE, 4 * sizeof(float), (void *) 0); 

	glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, 
        GL_FALSE, 4 * sizeof(float), (void *) (2*sizeof(float))); 
	
	glBindVertexArray(0);
}

void A5::init_textures() {
	m_rt_shader.enable();
		ubackground = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "bg_tex");
		glUniform1i(ubackground, 5);
		
		glActiveTexture(GL_TEXTURE5);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		int code = __loadTextureRGBA(m_bg_texture,getAssetFilePath("background.png").c_str());
		if (code < 0) {
			return;
		}

	m_rt_shader.disable();
	CHECK_GL_ERRORS;
}

void A5::init_uniforms() {
	m_rt_shader.enable();
		uobj = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "obj_tex");
		uvert = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "vert_tex");
		uelem = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "elem_tex");
		ubvhTex = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "bvh_tex");
		ubvhMeshTex = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "bvh_mesh_tex");

		uperlin = glGetUniformBlockIndex(m_rt_shader.getProgramObject(), "PerlinNoise");
		umat = glGetUniformBlockIndex(m_rt_shader.getProgramObject(), "Material");
		ubvh = glGetUniformBlockIndex(m_rt_shader.getProgramObject(), "BVH");
		ubvhMesh = glGetUniformBlockIndex(m_rt_shader.getProgramObject(), "BVHMesh");
		ulight = glGetUniformBlockIndex(m_rt_shader.getProgramObject(), "Light");
		ucamera = glGetUniformBlockIndex(m_rt_shader.getProgramObject(), "Camera");
	
		glUniform1i(uobj, 0);
		glUniform1i(uvert, 1);
		glUniform1i(uelem, 2);
		glUniform1i(ubvhTex, 3);
		glUniform1i(ubvhMeshTex, 4);

		glUniformBlockBinding(m_rt_shader.getProgramObject(),  uperlin, 0);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  umat, 1);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvh, 2);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvhMesh, 3);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ulight, 4);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ucamera, 5);
		
		{
			glGenBuffers(1, &m_camera);
			glBindBuffer(GL_UNIFORM_BUFFER, m_camera);
			glBufferData(GL_UNIFORM_BUFFER, 80, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glBindBufferRange(GL_UNIFORM_BUFFER, 5, m_camera, 0, 80);

			glBindBuffer(GL_UNIFORM_BUFFER, m_camera);

			glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(camera.getViewMatrix()));
			glBufferSubData(GL_UNIFORM_BUFFER, 64, 12, glm::value_ptr(camera.getCamEye()));
			int fov = camera.getFov();
			glBufferSubData(GL_UNIFORM_BUFFER, 76, 4, &fov);  
			glBindBuffer(GL_UNIFORM_BUFFER, 0);        
        }

		UboConstructor::construct(
			m_ubo_obj,
			m_ubo_vert,
			m_ubo_elem,
			m_ubo_bvh_tex,
			m_ubo_bvh_mesh_tex,
			m_ubo_perlin,
			m_ubo_mat,
			m_ubo_bvh,
			m_ubo_bvh_mesh,
			m_ubo_light
		);

		uambient = glGetUniformLocation(m_rt_shader.getProgramObject(), "ambient");
		unol = glGetUniformLocation(m_rt_shader.getProgramObject(), "num_of_lights");
		uwsize = glGetUniformLocation(m_rt_shader.getProgramObject(), "window_size");
		utotalArea = glGetUniformLocation(m_rt_shader.getProgramObject(), "total_light_area");
		useed = glGetUniformLocation(m_rt_shader.getProgramObject(), "seed");
	m_rt_shader.disable();
	CHECK_GL_ERRORS;
}

void A5::init() {
	glClearColor( ambient.x, ambient.y, ambient.z, 1.0 );
	// glfwWindowHint(GLFW_SAMPLES, 32);
	// glEnable(GL_MULTISAMPLE);
    init_shaders();
	m_rt_shader.enable();
	init_quad();
    init_textures();
    init_uniforms();
}

void A5::appLogic()
{

	float dtime = glfwGetTime() - state.prev_time_app;
	float delta = dtime * CAMERA_TRANSLATION_SPEED;
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.translate(Camera::TRANSLATION::FORWARD, delta);
	} else if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.translate(Camera::TRANSLATION::LEFT, delta);
	} else if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.translate(Camera::TRANSLATION::BACKWARD, delta);
	} else if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.translate(Camera::TRANSLATION::RIGHT, delta);
	}

	state.prev_time_app = glfwGetTime();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A5::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

void A5::draw() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	m_rt_shader.enable();
		glUniform1i(uobj, 0);
		glUniform1i(uvert, 1);
		glUniform1i(uelem, 2);
		glUniform1i(ubvhTex, 3);
		glUniform1i(ubvhMeshTex, 4);
		glUniform1i(ubackground, 5);

		glUniformBlockBinding(m_rt_shader.getProgramObject(),  uperlin, 0);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  umat, 1);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvh, 2);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvhMesh, 3);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ulight, 4);

		glUniform3fv(uambient, 1, glm::value_ptr(ambient));
		int size = UboConstructor::light_arr.size();
		glUniform1i(unol, size);
		glUniform2f(uwsize,m_framebufferWidth, m_framebufferHeight);
		// generate random seeds
		glm::vec4 time = glm::vec4(glfwGetTime(), 0,0,0);
		glUniform4fv(useed, 1, glm::value_ptr(time));
		glUniform1fv(utotalArea, 1, &total_area);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ubo_obj);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_ubo_vert);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_ubo_elem);

		if (UboConstructor::bvh_arr.size() > 1024) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, m_ubo_bvh_tex);
		}

		if (UboConstructor::bvh_mesh_arr.size() > 1024) {
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, m_ubo_bvh_mesh_tex);
		}
		
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_bg_texture);
		
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_perlin);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_mat);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh_mesh);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_light);

		glBindBuffer(GL_UNIFORM_BUFFER, m_camera);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(camera.getViewMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 12, glm::value_ptr(camera.getCamEye()));
		float fov = camera.getFov();
		glBufferSubData(GL_UNIFORM_BUFFER, 76, 4, &fov);
		glBindBuffer(GL_UNIFORM_BUFFER, 0); 


		glBindVertexArray(m_vao_quad);
		for (int i = 0; i < SPP; ++i) {
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		CHECK_GL_ERRORS;
	m_rt_shader.disable();
}

void A5::cleanup() {
	
}

bool A5::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);
	return eventHandled;
}

bool A5::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		float currentTime = glfwGetTime();
        float dtime = glfwGetTime() - state.prev_time_mouse;

		if (!state.is_mouse_pos_init) {
			state.mouse_pos_prev = glm::vec2(xPos, yPos);
			state.is_mouse_pos_init = true;
		} 

		float dx = xPos - state.mouse_pos_prev.x;
		float dy = yPos - state.mouse_pos_prev.y;

		if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			float sensitivity = 0.03;
			camera.rotate(Camera::ROTATION::YAW, CAMERA_ROTATE_SPEED*dx*dtime);
			camera.rotate(Camera::ROTATION::PITCH, -CAMERA_ROTATE_SPEED*dy*dtime);
		}

		state.mouse_pos_prev = glm::vec2(xPos, yPos);

		state.prev_time_mouse = currentTime;
	}

	return eventHandled;
}

bool A5::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);
	camera.zoom(-(float)yOffSet);
	return true;
}

bool A5::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}


bool A5::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	return eventHandled;
}

bool A5::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}


A5::~A5() {
	glDeleteVertexArrays(1, &m_vao_quad);
	glDeleteBuffers(1, &m_vbo_quad);

	glDeleteBuffers(1, &m_ubo_perlin);
	glDeleteBuffers(1, &m_ubo_mat);
	glDeleteBuffers(1, &m_ubo_bvh);
	glDeleteBuffers(1, &m_ubo_bvh_mesh);
	glDeleteBuffers(1, &m_ubo_light);
	glDeleteBuffers(1, &m_camera);

	glDeleteTextures(1, &m_ubo_obj);
	glDeleteTextures(1, &m_ubo_vert);
	glDeleteTextures(1, &m_ubo_elem);
	glDeleteTextures(1, &m_ubo_bvh_tex);
	glDeleteTextures(1, &m_ubo_bvh_mesh_tex);
	glDeleteTextures(1, &m_bg_texture);
	
	
}