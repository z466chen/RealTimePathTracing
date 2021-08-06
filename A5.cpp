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

#define SPP 2

const float CAMERA_TRANSLATION_SPEED = 800.0f;
const float CAMERA_ROTATE_SPEED = 9.0f;
const int MAX_FRAME_DELAY = 1;
const int SPHERE_ID_1 = 2;

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

void A5::camera_update(GLuint camera_id) {
	glBindBuffer(GL_UNIFORM_BUFFER, camera_id);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(camera.getViewMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, 64, 12, glm::value_ptr(camera.getCamEye()));
	float fov = camera.getFov();
	glBufferSubData(GL_UNIFORM_BUFFER, 76, 4, &fov);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); 
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

// void A5::init_mesh_gb() {
// 	for (auto id: UboConstructor::mesh_obj_ids) {
// 		UboObject &obj = UboConstructor::obj_arr[id];
// 		int voffset = int(obj.obj_data_1.y);
// 		int vlength = int(obj.obj_data_2.x);
// 		if (m_mbo_gb.find(voffset) == m_mbo_gb.end()) {

// 			m_mbo_gb.emplace(voffset, MeshBufferObject());
// 			MeshBufferObject &mbo = m_mbo_gb[voffset];

// 			glGenBuffers(1, &mbo.m_vbo_pos_gb);
// 			glBindBuffer(GL_ARRAY_BUFFER, mbo.m_vbo_pos_gb);
// 			glBufferData(GL_ARRAY_BUFFER, 12*vlength, 
// 				&UboConstructor::vert_arr[voffset], GL_STATIC_DRAW);

// 			glGenBuffers(1, &mbo.m_vbo_normal_gb);
// 			glBindBuffer(GL_ARRAY_BUFFER, mbo.m_vbo_normal_gb);
// 			glBufferData(GL_ARRAY_BUFFER, 12*vlength, 
// 				&UboConstructor::normal_arr[voffset], GL_STATIC_DRAW);

// 			// Create VAO
// 			glGenVertexArrays(1, &mbo.m_vao_mesh_gb);
// 			glBindVertexArray(mbo.m_vao_mesh_gb);

// 			// Create VBO
			

// 			// Attach Vertex Attributes
// 			glBindBuffer(GL_ARRAY_BUFFER, mbo.m_vbo_pos_gb);
// 			glVertexAttribPointer( 0, 3, GL_FLOAT, 
// 				GL_FALSE, 3 * sizeof(float), (void *) 0); 
// 			glEnableVertexAttribArray( 0 );

// 			glBindBuffer(GL_ARRAY_BUFFER, mbo.m_vbo_normal_gb);
// 			glVertexAttribPointer( 1, 3, GL_FLOAT, 
// 				GL_FALSE, 3 * sizeof(float), (void *) 0); 
// 			glEnableVertexAttribArray( 1 );
			
// 			glBindVertexArray(0);	

// 			CHECK_GL_ERRORS;
// 		}
		
// 	}
// }

void A5::init_shaders() {
	UboConstructor::calcDefines();
	isb_shader_object.create();
	trb_shader_object.create();
	srb_shader_object.create();
	ig_shader_object.create();
}

void A5::init_textures() {
	glActiveTexture(GL_TEXTURE5);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int code = __loadTextureRGBA(m_bump_map,getAssetFilePath("bump_map.png").c_str());
	if (code < 0) {
		return;
	}
	

	init_fbo(fbos[0], 7);
	init_fbo(fbos[1], 7);
	init_fbo(fbos[2], 7);
	init_fbo(fbos[3], 7);
	init_fbo(fbos[4], 7);
	init_fbo(fbos[5], 2);
	init_fbo(fbos[6], 2);
	CHECK_GL_ERRORS;
}

void A5::init_uniforms() {
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
	CHECK_GL_ERRORS;
}

void A5::init_fbo(FrameBufferObject &obj, int number) {
	glGenFramebuffers(1, &obj.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, obj.fbo);
	

	obj.number = number;
	std::vector<GLuint> attachment_points;
	for(int i = 0; i < number; ++i) {
		glGenTextures(1, &obj.data_objs[i]);
		glBindTexture(GL_TEXTURE_2D, obj.data_objs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 
			m_framebufferWidth, m_framebufferHeight, 0, GL_RGBA, GL_FLOAT, 0);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, 
			GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, obj.data_objs[i], 0);
		attachment_points.emplace_back(GL_COLOR_ATTACHMENT0+i);
	}
	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	glDrawBuffers(number, &attachment_points[0]);
	
    // glGenRenderbuffers(1, &obj.depth);
    // glBindRenderbuffer(GL_RENDERBUFFER, obj.depth);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_framebufferWidth, m_windowHeight);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, obj.depth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECK_GL_ERRORS;
}

void A5::use_fbo(FrameBufferObject &obj, GLuint TexOffset, int number) {
	for (int i = 0; i < number; ++i) {
		glActiveTexture(TexOffset+i);
		glBindTexture(GL_TEXTURE_2D, obj.data_objs[i]);
	}
} 

void A5::attach_fbo(FrameBufferObject &obj) {
	glBindFramebuffer(GL_FRAMEBUFFER, obj.fbo);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void A5::init() {
	glClearColor( ambient.x, ambient.y, ambient.z, 1.0 );

	glfwSwapInterval(0);
	// glfwWindowHint(GLFW_SAMPLES, 32);
	// glEnable(GL_MULTISAMPLE);
    init_shaders();
	init_quad();
	// init_mesh_gb();
    init_textures();
    init_uniforms();
}

void A5::update_position(int oid, int axis, glm::vec3 delta) {
	UboObject &obj = UboConstructor::obj_arr[oid];
	obj.t_matrix = glm::translate(obj.t_matrix,delta);
	obj.inv_t_matrix = glm::inverse(obj.t_matrix);
	int bvh_node_id = UboConstructor::obj_bvh_reference[oid];
	
	AABB bbox;
	bbox.lower_bound = glm::vec3(obj.obj_aabb_1.x, obj.obj_aabb_1.y, obj.obj_aabb_2.x) + delta;
	bbox.upper_bound = glm::vec3(obj.obj_aabb_2.y, obj.obj_aabb_3.x, obj.obj_aabb_3.y) + delta;

	obj.obj_aabb_1 = glm::vec2(bbox.lower_bound.x, bbox.lower_bound.y);
	obj.obj_aabb_2 = glm::vec2(bbox.lower_bound.z, bbox.upper_bound.x);
	obj.obj_aabb_3 = glm::vec2( bbox.upper_bound.y,  bbox.upper_bound.z);
	
	int currentid = bvh_node_id;
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo_bvh);
	while(currentid >= 0) {
		UboBVH &bvh = UboConstructor::bvh_arr[currentid];
		AABB bvhbox;
		bvhbox.lower_bound = glm::vec3(bvh.bvh_aabb_1.x, bvh.bvh_aabb_1.y, bvh.bvh_aabb_2.x);
		bvhbox.upper_bound = glm::vec3(bvh.bvh_aabb_2.y, bvh.bvh_aabb_3.x, bvh.bvh_aabb_3.y);
		bvhbox = bbox + bvhbox;
		bvh.bvh_aabb_1 = glm::vec2(bvhbox.lower_bound.x, bvhbox.lower_bound.y);
		bvh.bvh_aabb_2 = glm::vec2(bvhbox.lower_bound.z, bvhbox.upper_bound.x);
		bvh.bvh_aabb_3 = glm::vec2( bvhbox.upper_bound.y,  bvhbox.upper_bound.z);
		int i = currentid;
        glBufferSubData(GL_UNIFORM_BUFFER, i*48+0, 8, glm::value_ptr(bvh.bvh_aabb_1));
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+8, 8, glm::value_ptr(bvh.bvh_aabb_2));
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+16, 8, glm::value_ptr(bvh.bvh_aabb_3));
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+24, 4, &bvh.bvh_left);
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+28, 4, &bvh.bvh_right);
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+32, 4, &bvh.obj_id_1);
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+36, 4, &bvh.obj_id_2);
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+40, 4, &bvh.obj_id_3);
		glBufferSubData(GL_UNIFORM_BUFFER, i*48+44, 4, &bvh.obj_id_4); 
        currentid = bvh.obj_id_3;
	}
	CHECK_GL_ERRORS;
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, m_ubo_obj);
	float temp[46];
	memcpy(temp, &obj, sizeof(float)*46);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 92, 0, 46, 
		1, GL_RED, GL_FLOAT, temp);
	glBindTexture(GL_TEXTURE_2D,0);
	CHECK_GL_ERRORS;
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

	float current = glfwGetTime();

	const int start_drop_distance = 180;
	// if (current >= 20 && current <= 20+((1+state.rho)/(1-state.rho))*sqrt(2*180.0f/9.8f)) {
	// 	float delta_t = (current - state.prev_time_app);
	// 	update_position(2, 1, glm::vec3(0,state.v*delta_t,0));
	// 	state.x = state.x + state.v*delta_t;
	// 	if (state.x < 0.01) {
	// 		state.v = state.rho * (-state.v);
	// 	} else {
	// 		state.v = state.v - state.g*delta_t;
	// 	}
	// 	camera.cameraChanged = true;
	// }
	state.prev_time_app = current;
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
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	std::vector<int> frame_buffer_sequence;
	if (state.frame_buffer_mode == 0) {
		frame_buffer_sequence = {4,2,0,3,1,5,6};
		state.frame_buffer_mode = 1;
	} else {
		frame_buffer_sequence = {4,0,2,1,3,6,5};
		state.frame_buffer_mode = 0;
	}

	bool camera_moving = camera.isCameraChanged();
	if (camera_moving) {
		state.frame_initialized = false;
		// clean all buffer data if camera changed
		for (int i = 0; i < 7; ++i) {
			attach_fbo(fbos[frame_buffer_sequence[i]]);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		camera.resetCameraChangedStatus();
	}
	glEnable(GL_DEPTH_TEST);
	glm::vec4 seed = glm::vec4(get_random_float(), get_random_float(),get_random_float(),get_random_float());



	{
		attach_fbo(fbos[frame_buffer_sequence[0]]);
		isb_shader_object.shader.enable();
			isb_shader_object.bindObjects();
			// use_fbo(fbos[frame_buffer_sequence[0]], GL_TEXTURE6, 3);
			camera_update(m_camera);
			int size = UboConstructor::light_arr.size();
			glUniform1i(isb_shader_object.unol, size);
			glUniform2f(isb_shader_object.uwsize,m_framebufferWidth, m_framebufferHeight);
			// generate random seeds
			
			glUniform4fv(isb_shader_object.useed, 1, glm::value_ptr(seed));
			glUniform1fv(isb_shader_object.utotalArea, 1, &total_area);
			glBindVertexArray(m_vao_quad);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			CHECK_GL_ERRORS;
		isb_shader_object.shader.disable();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	{
		attach_fbo(fbos[frame_buffer_sequence[2]]);
		trb_shader_object.shader.enable();
			trb_shader_object.bindObjects();
			use_fbo(fbos[frame_buffer_sequence[0]], GL_TEXTURE0, 6);
			use_fbo(fbos[frame_buffer_sequence[1]], GL_TEXTURE7, 7);
			glUniform4fv(trb_shader_object.useed, 1, glm::value_ptr(seed));
			glUniform1i(trb_shader_object.uinitialized, state.frame_initialized);
			glUniform1i(trb_shader_object.umbfc, state.motion_blur_frame_count);
			glUniform1i(trb_shader_object.umoving, camera_moving);
			glBindVertexArray(m_vao_quad);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			CHECK_GL_ERRORS;
		trb_shader_object.shader.disable();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (!state.frame_initialized) {
			state.frame_initialized = true;
		}
	}

	{
		attach_fbo(fbos[frame_buffer_sequence[4]]);
		srb_shader_object.shader.enable();
			srb_shader_object.bindObjects();
			use_fbo(fbos[frame_buffer_sequence[2]], GL_TEXTURE5, 7);
			use_fbo(fbos[frame_buffer_sequence[3]], GL_TEXTURE12, 7);
			camera_update(m_camera);
			glUniform2f(srb_shader_object.uwsize,m_framebufferWidth, m_framebufferHeight);
			glUniform4fv(srb_shader_object.useed, 1, glm::value_ptr(seed));
			glBindVertexArray(m_vao_quad);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			CHECK_GL_ERRORS;
		srb_shader_object.shader.disable();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	{
		attach_fbo(fbos[frame_buffer_sequence[6]]);
		ig_shader_object.shader.enable();
			ig_shader_object.bindObjects();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,fbos[frame_buffer_sequence[0]].data_objs[6]);
			use_fbo(fbos[frame_buffer_sequence[4]], GL_TEXTURE1, 7);
			use_fbo(fbos[frame_buffer_sequence[5]], GL_TEXTURE8, 2);
			glBindVertexArray(m_vao_quad);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			CHECK_GL_ERRORS;
		ig_shader_object.shader.disable();
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, m_framebufferWidth, m_framebufferHeight,
			0, 0, m_framebufferWidth, m_framebufferHeight,
              GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	state.motion_blur_frame_count = (state.motion_blur_frame_count+1)%MAX_FRAME_DELAY;

	CHECK_GL_ERRORS;
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


// Shaders 


void A5::ISBShaderObject::create() {
	shader.generateProgramObject();

	std::string frag_shader_src;
	frag_shader_src = readFile(getAssetFilePath(fs_name));
	
	replace(frag_shader_src, "{OBJ_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.obj_texture_size));
	replace(frag_shader_src, "{VERT_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.vert_texture_size));
	replace(frag_shader_src, "{ELEM_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.elem_texture_size));
	replace(frag_shader_src, "{BVH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_texture_size));
	replace(frag_shader_src, "{BVH_MESH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_mesh_texture_size));

	std::ofstream ofs;
	ofs.open (getAssetFilePath("shaders/temp.fs"), std::ofstream::out);
	ofs << frag_shader_src;
	ofs.close();

	shader.attachVertexShader(
		getAssetFilePath(vs_name).c_str()
	);
    shader.attachFragmentShader(
        getAssetFilePath("shaders/temp.fs").c_str()
    );

    shader.link();

	shader.enable();
		// texture objects
		uobj = glGetUniformLocation(
			shader.getProgramObject(), "obj_tex");
		uvert = glGetUniformLocation(
			shader.getProgramObject(), "vert_tex");
		uelem = glGetUniformLocation(
			shader.getProgramObject(), "elem_tex");
		ubvhTex = glGetUniformLocation(
			shader.getProgramObject(), "bvh_tex");
		ubvhMeshTex = glGetUniformLocation(
			shader.getProgramObject(), "bvh_mesh_tex");
		ubump_map_tex = glGetUniformLocation(
			shader.getProgramObject(), "bump_map_tex");
		// uPrevPosTex = glGetUniformLocation(
		// 	shader.getProgramObject(), "indata1");
		// uPrevNormalTex = glGetUniformLocation(
		// 	shader.getProgramObject(), "indata2");
		// uPrevIdTex = glGetUniformLocation(
		// 	shader.getProgramObject(), "indata3");


		// uniforms
		uambient = glGetUniformLocation(
			shader.getProgramObject(), "ambient");
		unol = glGetUniformLocation(
			shader.getProgramObject(), "num_of_lights");
		utotalArea = glGetUniformLocation(
			shader.getProgramObject(), "total_light_area");
		useed = glGetUniformLocation(
			shader.getProgramObject(), "seed");
		uwsize = glGetUniformLocation(
			shader.getProgramObject(), "window_size");

		// uniform objects
		uperlin = glGetUniformBlockIndex(shader.getProgramObject(), "PerlinNoise");
		umat = glGetUniformBlockIndex(shader.getProgramObject(), "Material");
		ubvh = glGetUniformBlockIndex(shader.getProgramObject(), "BVH");
		ubvhMesh = glGetUniformBlockIndex(shader.getProgramObject(), "BVHMesh");
		ulight = glGetUniformBlockIndex(shader.getProgramObject(), "Light");
		ucamera = glGetUniformBlockIndex(shader.getProgramObject(), "Camera");
	shader.disable();
	CHECK_GL_ERRORS;
}


void A5::ISBShaderObject::bindObjects() {
	glUniform1i(uobj, 0);
	glUniform1i(uvert, 1);
	glUniform1i(uelem, 2);
	glUniform1i(ubvhTex, 3);
	
	glUniform1i(ubvhMeshTex, 4);
	glUniform1i(ubump_map_tex, 5);
	
	// glUniform1i(ubackground, 5);
	// glUniform1i(uPrevPosTex, 6);
	// glUniform1i(uPrevNormalTex, 7);
	// glUniform1i(uPrevIdTex, 8);

	glUniformBlockBinding(shader.getProgramObject(),  uperlin, 0);
	glUniformBlockBinding(shader.getProgramObject(),  umat, 1);
	glUniformBlockBinding(shader.getProgramObject(),  ubvh, 2);
	glUniformBlockBinding(shader.getProgramObject(),  ubvhMesh, 3);
	glUniformBlockBinding(shader.getProgramObject(),  ulight, 4);
	glUniformBlockBinding(shader.getProgramObject(),  ucamera, 5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, parent.m_ubo_obj);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, parent.m_ubo_vert);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, parent.m_ubo_elem);	

	if (UboConstructor::bvh_arr.size() > 1024) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, parent.m_ubo_bvh_tex);
	}

	if (UboConstructor::bvh_mesh_arr.size() > 1024) {
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, parent.m_ubo_bvh_mesh_tex);
	}

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, parent.m_bump_map);
	
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_perlin);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_mat);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_bvh);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_bvh_mesh);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_light);
	CHECK_GL_ERRORS;
}



void A5::TRBShaderObject::create() {
	shader.generateProgramObject();

	std::string frag_shader_src;
	frag_shader_src = readFile(getAssetFilePath(fs_name));
	
	// replace(frag_shader_src, "{OBJ_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.obj_texture_size));
	// replace(frag_shader_src, "{VERT_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.vert_texture_size));
	// replace(frag_shader_src, "{ELEM_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.elem_texture_size));
	// replace(frag_shader_src, "{BVH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_texture_size));
	// replace(frag_shader_src, "{BVH_MESH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_mesh_texture_size));

	std::ofstream ofs;
	ofs.open (getAssetFilePath("shaders/temp.fs"), std::ofstream::out);
	ofs << frag_shader_src;
	ofs.close();

	shader.attachVertexShader(
		getAssetFilePath(vs_name).c_str()
	);
    shader.attachFragmentShader(
        getAssetFilePath("shaders/temp.fs").c_str()
    );

    shader.link();

	shader.enable();

		uisb_tex1 = glGetUniformLocation(
			shader.getProgramObject(), "isb_tex1");
		uisb_tex2 = glGetUniformLocation(
			shader.getProgramObject(), "isb_tex2");
		uisb_tex3 = glGetUniformLocation(
			shader.getProgramObject(), "isb_tex3");
		uisb_tex4 = glGetUniformLocation(
			shader.getProgramObject(), "isb_tex4");
		uisb_tex5 = glGetUniformLocation(
			shader.getProgramObject(), "isb_tex5");
		uisb_tex6 = glGetUniformLocation(
			shader.getProgramObject(), "isb_tex6");
		
		utrb_tex1 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex1");
		utrb_tex2 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex2");
		utrb_tex3 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex3");
		utrb_tex4 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex4");
		utrb_tex5 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex5");
		utrb_tex6 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex6");
		utrb_tex7 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex7");
			
		useed = glGetUniformLocation(
			shader.getProgramObject(), "seed");
		uinitialized = glGetUniformLocation(
			shader.getProgramObject(), "initialized");
		umbfc = glGetUniformLocation(
			shader.getProgramObject(), "mbfc");
		umoving = glGetUniformLocation(
			shader.getProgramObject(), "moving");
	shader.disable();
	CHECK_GL_ERRORS;
}


void A5::TRBShaderObject::bindObjects() {
	glUniform1i(uisb_tex1, 0);
	glUniform1i(uisb_tex2, 1);
	glUniform1i(uisb_tex3, 2);
	glUniform1i(uisb_tex4, 3);
	glUniform1i(uisb_tex5, 4);
	glUniform1i(uisb_tex6, 5);

	glUniform1i(utrb_tex1, 7);
	glUniform1i(utrb_tex2, 8);
	glUniform1i(utrb_tex3, 9);
	glUniform1i(utrb_tex4, 10);
	glUniform1i(utrb_tex5, 11);
	glUniform1i(utrb_tex6, 12);	
	glUniform1i(utrb_tex7, 13);	
}



void A5::SRBShaderObject::create() {
	shader.generateProgramObject();

	std::string frag_shader_src;
	frag_shader_src = readFile(getAssetFilePath(fs_name));
	
	replace(frag_shader_src, "{OBJ_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.obj_texture_size));
	replace(frag_shader_src, "{VERT_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.vert_texture_size));
	replace(frag_shader_src, "{ELEM_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.elem_texture_size));
	replace(frag_shader_src, "{BVH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_texture_size));
	replace(frag_shader_src, "{BVH_MESH_TEXTURE_SIZE}", std::to_string(UboConstructor::defines.bvh_mesh_texture_size));

	std::ofstream ofs;
	ofs.open (getAssetFilePath("shaders/temp.fs"), std::ofstream::out);
	ofs << frag_shader_src;
	ofs.close();

	shader.attachVertexShader(
		getAssetFilePath(vs_name).c_str()
	);
    shader.attachFragmentShader(
        getAssetFilePath("shaders/temp.fs").c_str()
    );

    shader.link();

	shader.enable();
		// texture objects
		uobj = glGetUniformLocation(
			shader.getProgramObject(), "obj_tex");
		uvert = glGetUniformLocation(
			shader.getProgramObject(), "vert_tex");
		uelem = glGetUniformLocation(
			shader.getProgramObject(), "elem_tex");
		ubvhTex = glGetUniformLocation(
			shader.getProgramObject(), "bvh_tex");
		ubvhMeshTex = glGetUniformLocation(
			shader.getProgramObject(), "bvh_mesh_tex");


		// uniforms
		useed = glGetUniformLocation(
			shader.getProgramObject(), "seed");
		uwsize = glGetUniformLocation(
			shader.getProgramObject(), "window_size");

		// uniform objects
		uperlin = glGetUniformBlockIndex(shader.getProgramObject(), "PerlinNoise");
		umat = glGetUniformBlockIndex(shader.getProgramObject(), "Material");
		ubvh = glGetUniformBlockIndex(shader.getProgramObject(), "BVH");
		ubvhMesh = glGetUniformBlockIndex(shader.getProgramObject(), "BVHMesh");
		ulight = glGetUniformBlockIndex(shader.getProgramObject(), "Light");
		ucamera = glGetUniformBlockIndex(shader.getProgramObject(), "Camera");
	
		usrb_tex1 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex1");
		usrb_tex2 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex2");
		usrb_tex3 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex3");
		usrb_tex4 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex4");
		usrb_tex5 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex5");
		usrb_tex6 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex6");
		usrb_tex7 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex7");
		
		utrb_tex1 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex1");
		utrb_tex2 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex2");
		utrb_tex3 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex3");
		utrb_tex4 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex4");
		utrb_tex5 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex5");
		utrb_tex6 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex6");
		utrb_tex7 = glGetUniformLocation(
			shader.getProgramObject(), "trb_tex7");
	shader.disable();
	CHECK_GL_ERRORS;
}


void A5::SRBShaderObject::bindObjects() {
	glUniform1i(uobj, 0);
	glUniform1i(uvert, 1);
	glUniform1i(uelem, 2);
	glUniform1i(ubvhTex, 3);
	glUniform1i(ubvhMeshTex, 4);
	glUniform1i(usrb_tex1, 5);
	glUniform1i(usrb_tex2, 6);
	glUniform1i(usrb_tex3, 7);
	glUniform1i(usrb_tex4, 8);
	glUniform1i(usrb_tex5, 9);
	glUniform1i(usrb_tex6, 10);
	glUniform1i(usrb_tex7, 11);

	glUniform1i(utrb_tex1, 12);
	glUniform1i(utrb_tex2, 13);
	glUniform1i(utrb_tex3, 14);
	glUniform1i(utrb_tex4, 15);
	glUniform1i(utrb_tex5, 16);
	glUniform1i(utrb_tex6, 17);	
	glUniform1i(utrb_tex7, 18);	
	

	glUniformBlockBinding(shader.getProgramObject(),  uperlin, 0);
	glUniformBlockBinding(shader.getProgramObject(),  umat, 1);
	glUniformBlockBinding(shader.getProgramObject(),  ubvh, 2);
	glUniformBlockBinding(shader.getProgramObject(),  ubvhMesh, 3);
	glUniformBlockBinding(shader.getProgramObject(),  ulight, 4);
	glUniformBlockBinding(shader.getProgramObject(),  ucamera, 5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, parent.m_ubo_obj);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, parent.m_ubo_vert);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, parent.m_ubo_elem);	

	if (UboConstructor::bvh_arr.size() > 1024) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, parent.m_ubo_bvh_tex);
	}

	if (UboConstructor::bvh_mesh_arr.size() > 1024) {
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, parent.m_ubo_bvh_mesh_tex);
	}

	// glActiveTexture(GL_TEXTURE5);
	// glBindTexture(GL_TEXTURE_2D, parent.m_bg_texture);
	
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_perlin);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_mat);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_bvh);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_bvh_mesh);
	glBindBuffer(GL_UNIFORM_BUFFER, parent.m_ubo_light);
	CHECK_GL_ERRORS;
}


void A5::IGShaderObject::create() {
	shader.generateProgramObject();

	std::string frag_shader_src;
	frag_shader_src = readFile(getAssetFilePath(fs_name));

	std::ofstream ofs;
	ofs.open (getAssetFilePath("shaders/temp.fs"), std::ofstream::out);
	ofs << frag_shader_src;
	ofs.close();

	shader.attachVertexShader(
		getAssetFilePath(vs_name).c_str()
	);
    shader.attachFragmentShader(
        getAssetFilePath("shaders/temp.fs").c_str()
    );

    shader.link();

	shader.enable();

		uemission_tex = glGetUniformLocation(
			shader.getProgramObject(), "emission_tex");
		
		usrb_tex1 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex1");
		usrb_tex2 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex2");
		usrb_tex3 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex3");
		usrb_tex4 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex4");
		usrb_tex5 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex5");
		usrb_tex6 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex6");
		usrb_tex7 = glGetUniformLocation(
			shader.getProgramObject(), "srb_tex7");

		ucolor_buffer = glGetUniformLocation(
			shader.getProgramObject(), "color_buffer");
		unumber_buffer = glGetUniformLocation(
			shader.getProgramObject(), "number_buffer");

	shader.disable();
	CHECK_GL_ERRORS;
}


void A5::IGShaderObject::bindObjects() {
	glUniform1i(uemission_tex, 0);
	
	glUniform1i(usrb_tex1, 1);
	glUniform1i(usrb_tex2, 2);
	glUniform1i(usrb_tex3, 3);
	glUniform1i(usrb_tex4, 4);
	glUniform1i(usrb_tex5, 5);
	glUniform1i(usrb_tex6, 6);	
	glUniform1i(usrb_tex7, 7);	

	glUniform1i(ucolor_buffer, 8);
	glUniform1i(unumber_buffer, 9);
}
