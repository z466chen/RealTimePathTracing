// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "Camera.hpp"
#include "Ray.hpp"

class A4_Scene {

	bool initialized = false;


	glm::vec3 __RTCastRay(const Ray &ray, int depth) const;
public:
	SceneNode *root;

	Camera camera;

	glm::vec3 ambient;
	std::list<Light *> lights;

	static const int maxDepth = 5;

	A4_Scene(SceneNode *root, 
		glm::vec3 &eye, glm::vec3 &view, glm::vec3 &up, double fov, 
		glm::vec3 &ambient, std::list<Light *> &lights);

	void mark_initialized() { initialized = true; }
	bool is_initialized() const { return initialized; }


	Intersection traverse(const Ray & ray) const;
	glm::vec3 castRay(const Ray & ray, int depth) const;
};

class A4_Canvas {
public:
	Image image;
	A4_Canvas(uint width, uint height): image{width, height} {};
	void render(const A4_Scene & scene);
};

// void A4_Render(
// 		// What to render
// 		SceneNode * root,

// 		// Image to write to, set to a given width and height
// 		Image & image,

// 		// Viewing parameters
// 		const glm::vec3 & eye,
// 		const glm::vec3 & view,
// 		const glm::vec3 & up,
// 		double fovy,

// 		// Lighting parameters
// 		const glm::vec3 & ambient,
// 		const std::list<Light *> & lights
// );
