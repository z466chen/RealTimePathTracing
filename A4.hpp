// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "defines.hpp"

class A4_Scene {

	bool initialized = false;


	glm::vec3 __RTCastRay(const Ray &ray, int depth, const glm::vec3 &background_color) const;
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
	glm::vec3 castRay(const Ray & ray, int depth, const glm::vec3 &background_color) const;
};

class A4_Canvas {
public:
	static const char * background;
	Image background_img;
	Image image;
	A4_Canvas(uint width, uint height): image{width, height}, background_img{width, height, background} {};
	void render(const A4_Scene & scene);
};
