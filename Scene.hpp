// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "Camera.hpp"
#include "Ray.hpp"
#include "defines.hpp"
#include "BVH.hpp"
#include "Sampler.hpp"

class Scene {

	std::unique_ptr<BVH> bvh;

	glm::vec3 __RTCastRay(const Ray &ray, int depth) const;
public:
	SceneNode *root;

	Camera camera;

	glm::vec3 ambient;
	std::list<Light *> lights;

	static const int maxDepth = 5;
	

	Scene(SceneNode *root, 
		glm::vec3 &eye, glm::vec3 &view, glm::vec3 &up, double fov, 
		glm::vec3 &ambient, std::list<Light *> &lights);

	Intersection traverse(const Ray & ray) const;
	glm::vec3 castRay(const Ray & ray, int depth) const;
};