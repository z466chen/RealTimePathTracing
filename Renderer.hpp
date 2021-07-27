#pragma once

#include "Scene.hpp"

class Renderer {
public:
	static const char * background;

	const int sample_size = 2;
	SamplerType s_type = SamplerType::SINGLE;
	std::unique_ptr<Sampler> sampler = std::make_unique<SingleSampler>();
	Image image;
	Image bg_img;
	

	Renderer(uint width, uint height): image{width, height}, bg_img{width, height,background} {};
	Renderer(uint width, uint height, SamplerType s_type): Renderer{width, height}{
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
	
	void render(const Scene & scene);
};
