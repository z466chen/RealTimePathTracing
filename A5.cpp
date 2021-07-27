#include "A5.hpp"
#include <imgui/imgui.h>
#include <lodepng/lodepng.h>
#include <iostream>
#include "UboConstructor.hpp"
#include "BVH.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

const float CAMERA_TRANSLATION_SPEED = 20.0f;
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
	bvh_id = bvh.construct();
	for (auto l: lights) {
		l->construct();
	}
}

void A5::init_shaders() {
	m_rt_shader.generateProgramObject();
	m_rt_shader.attachVertexShader(
		getAssetFilePath("shaders/Default.vs").c_str()
	);
    m_rt_shader.attachFragmentShader(
        getAssetFilePath("shaders/RayTrace.fs").c_str()
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
		glUniform1i(ubackground, 6);
		
		glActiveTexture(GL_TEXTURE6);
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
		uperlin = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "perlin_tex");
		ubvhTex = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "bvh_tex");
		ubvhMeshTex = glGetUniformLocation(
			m_rt_shader.getProgramObject(), "bvh_mesh_tex");

		glUniform1i(uobj, 0);
		glUniform1i(uvert, 1);
		glUniform1i(uelem, 2);
		glUniform1i(uperlin, 3);
		glUniform1i(ubvhTex, 4);
		glUniform1i(ubvhMeshTex, 5);

		glUniformBlockBinding(m_rt_shader.getProgramObject(),  umat, 0);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvh, 1);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvhMesh, 2);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ulight, 3);
		
		UboConstructor::construct(
			m_ubo_obj,
			m_ubo_vert,
			m_ubo_elem,
			m_ubo_perlin,
			m_ubo_bvh_tex,
			m_ubo_bvh_mesh_tex,
			m_ubo_mat,
			m_ubo_bvh,
			m_ubo_bvh_mesh,
			m_ubo_light
		);
	m_rt_shader.disable();
	CHECK_GL_ERRORS;
}

void A5::init() {
	glClearColor( ambient.x, ambient.y, ambient.z, 1.0 );
    init_shaders();
	m_rt_shader.enable();
	init_quad();
    init_textures();
    init_uniforms();
}

void A5::appLogic()
{

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
		glUniform1i(uperlin, 3);
		glUniform1i(ubvhTex, 4);
		glUniform1i(ubvhMeshTex, 5);
		glUniform1i(ubackground, 6);

		glUniformBlockBinding(m_rt_shader.getProgramObject(),  umat, 0);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvh, 1);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ubvhMesh, 2);
		glUniformBlockBinding(m_rt_shader.getProgramObject(),  ulight, 3);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_ubo_obj);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_ubo_vert);
		
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_ubo_elem);
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_ubo_perlin);
		
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_ubo_bvh_tex);
		
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_ubo_bvh_mesh_tex);
		
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, m_bg_texture);

		glBindVertexArray(m_vao_quad);
		glDrawArrays(GL_TRIANGLES, 0, 6);
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

        float dtime = glfwGetTime() - state.prev_time_app;

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
	}

	return eventHandled;
}

bool A5::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);
	return eventHandled;
}

bool A5::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}


bool A5::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
        float dtime = glfwGetTime() - state.prev_time_app;
        float delta = dtime * CAMERA_TRANSLATION_SPEED;
		if (key == GLFW_KEY_UP) {
			camera.translate(Camera::TRANSLATION::FORWARD, delta);	
		} else if (key == GLFW_KEY_LEFT) {
            camera.translate(Camera::TRANSLATION::LEFT, delta);
		} else if (key == GLFW_KEY_DOWN) {
			camera.translate(Camera::TRANSLATION::BACKWARD, delta);
		} else if (key == GLFW_KEY_RIGHT) {
			camera.translate(Camera::TRANSLATION::RIGHT, delta);
		}
	}
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

	glDeleteTextures(1, &m_ubo_obj);
	glDeleteTextures(1, &m_ubo_vert);
	glDeleteTextures(1, &m_ubo_elem);
	glDeleteTextures(1, &m_ubo_perlin);
	glDeleteTextures(1, &m_ubo_bvh_tex);
	glDeleteTextures(1, &m_ubo_bvh_mesh_tex);
	glDeleteTextures(1, &m_bg_texture);
	
	glDeleteBuffers(1, &m_ubo_mat);
	glDeleteBuffers(1, &m_ubo_bvh);
	glDeleteBuffers(1, &m_ubo_bvh_mesh);
	glDeleteBuffers(1, &m_ubo_light);
}