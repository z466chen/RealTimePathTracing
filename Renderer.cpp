#include "Renderer.hpp"
#include "general.hpp"
#include <glm/ext.hpp>

const char * Renderer::background = "background.png";

void Renderer::render(const Scene & scene) {
  	// Fill in raytracing code here...  

  	std::cout << "F20: Calling A4_Render(\n" <<
		  "\t" << *scene.root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(scene.camera.getCamEye()) << std::endl <<
		  "\t" << "view: " << glm::to_string(scene.camera.getCamView()) << std::endl <<
		  "\t" << "up:   " << glm::to_string(scene.camera.getCamUp()) << std::endl <<
		  "\t" << "fovy: " << scene.camera.getFov() << std::endl <<
          "\t" << "ambient: " << glm::to_string(scene.ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : scene.lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	float p = 0;

	double w_size = tan(glm::radians(scene.camera.getFov()));
	double h_ratio = w_size/h;
	double w_ratio = w_size/w;

	sampler->init(image);
	size_t nos = sampler->getNos();
	for (size_t i = 0; i < nos; ++i) {
		glm::vec2 sample(0.0f);
		float weight = 0.0f;
		std::vector<glm::ivec2> pixels;
		sampler->pick(sample, weight, pixels);
		glm::vec3 dir = vtrans(glm::transpose(scene.camera.getViewMatrix()), 
			glm::vec3((sample.x- w*0.5f)*h_ratio, (h*0.5f - sample.y)*w_ratio,-1.0f));
		
		Ray primaryRay = Ray(scene.camera.getCamEye(), dir);
		glm::vec3 color = scene.castRay(primaryRay, 0);

		for (auto &p: pixels) {
			// Red: 
			image(p.x, p.y, 0) += (double)color.r*weight;
			// Green: 
			image(p.x, p.y, 1) += (double)color.g*weight;
			// Blue: 
			image(p.x, p.y, 2) += (double)color.b*weight;			
		}

		for (auto &p: pixels) {
			if (glm::l2Norm(glm::vec3(image(p.x, p.y, 0), 
			image(p.x, p.y, 1), image(p.x, p.y, 2))) < EPSILON) {
				// Red: 
				image(p.x, p.y, 0) = (double)bg_img(p.x, p.y, 0);
				// Green: 
				image(p.x, p.y, 1) = (double)bg_img(p.x, p.y, 1);
				// Blue: 
				image(p.x, p.y, 2) = (double)bg_img(p.x, p.y, 2);	
			}
		}
	}
}
