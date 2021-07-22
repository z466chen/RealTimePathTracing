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

class A4_Scene {

	bool initialized = false;

	std::unique_ptr<BVH> bvh;

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
	static const char * background;

	#ifdef ADAPTIVE
		const float adaptive_thresh = 1;
	#endif

	const int sample_size = 2;
	SamplerType s_type = SamplerType::SINGLE;
	std::unique_ptr<Sampler> sampler = std::make_unique<SingleSampler>();
	Image image;
	Image bg_img;
	

	A4_Canvas(uint width, uint height): image{width, height}, bg_img{width, height,background} {};
	A4_Canvas(uint width, uint height, SamplerType s_type): A4_Canvas{width, height}{
		this->s_type = s_type;
		switch (s_type) {

		case SamplerType::SINGLE :
			sampler = std::make_unique<SingleSampler>();
			break;

		case SamplerType::SS_GRID :
			sampler = std::make_unique<SuperSamplerGrid>(sample_size);
			break;
		
		case SamplerType::SS_JITTER :
			sampler = std::make_unique<SuperSamplerJitter>(sample_size);
			break;
		
		case SamplerType::SS_QUINCUNX :
			sampler = std::make_unique<SuperSamplerQuincunx>();
			break;
		
		case SamplerType::SS_RANDOM :
			sampler = std::make_unique<SuperSamplerRandom>(sample_size);
			break;
		
		case SamplerType::SS_RG :
			sampler = std::make_unique<SuperSamplerRotatedGrid>(sample_size);
			break;
		
		case SamplerType::SS_QMC :
			sampler = std::make_unique<SuperSamplerQMC>(sample_size);
			break;

		default:
			break;
		}
	}
	
	void render(const A4_Scene & scene);
};
