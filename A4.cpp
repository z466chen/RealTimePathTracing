// Termm--Fall 2020

#include <glm/ext.hpp>
#include "A4.hpp"
#include "PhongMaterial.hpp"
#include <queue>
#include <utility>
#include "general.hpp"

const char * A4_Canvas::background = "background.png";

A4_Scene::A4_Scene(SceneNode *root, 
		glm::vec3 &eye, glm::vec3 &view, glm::vec3 &up, double fov, 
		glm::vec3 &ambient, std::list<Light *> &lights): root{root}, 
		camera{(float)fov, eye,glm::normalize(view - eye),  glm::normalize(up)},
		ambient{ambient}, lights{lights}, initialized{true} {
	bvh = std::make_unique<BVH>(root);
}

Intersection A4_Scene::traverse(const Ray &ray) const {
	return bvh->intersect(ray);
}

glm::vec3 A4_Scene::__RTCastRay(const Ray &ray,int depth) const {
	glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
	if (depth > maxDepth) return glm::vec3(0.0f); 
	Intersection payload = traverse(ray);

	
	if (payload.intersects) {
		
		const PhongMaterialInfo *matInfo = static_cast<const PhongMaterialInfo *>(payload.matInfo.get());

		switch(matInfo->type) {
			case MaterialType::SPECULAR: {
 				double cos_i = glm::dot(-ray.direction, payload.normal);

				double theta_t;
				double kr;
				double n1 = 1.0f;
				double n2 = matInfo->ior;
				if (cos_i < 0) {
					std::swap(n1, n2);
				}
				cos_i = fabs(cos_i);
				double theta_i = acos(cos_i);

				matInfo->snell(theta_i, theta_t, n1, n2);
				matInfo->fresnel(theta_i, theta_t, kr, n1, n2);

				glm::vec3 reflectDir = glm::normalize(2.0f * cos_i * payload.normal + ray.direction);
				glm::vec3 refractDir = glm::normalize(((n1/n2) * cos_i - cos(theta_t)) * payload.normal + 
					(n1/n2) * ray.direction);

				Ray reflectionRay = Ray(payload.position + 
					((glm::dot(payload.normal, reflectDir) > 0)? payload.normal*EPSILON: 
					-payload.normal*EPSILON), reflectDir);
			
				Ray refractionRay = Ray(payload.position + 
					((glm::dot(payload.normal, refractDir) > 0)? payload.normal*EPSILON: 
					-payload.normal*EPSILON), refractDir);

				return vec_min(ambient * matInfo->kd + 
						matInfo->ks * (kr * castRay(reflectionRay,depth+1)  
					 +(1 - kr) * castRay(refractionRay,depth+1)), white);
				break;
			}
			case MaterialType::DIFFUSE: {

				glm::vec3 specular(0.0f);
				glm::vec3 diffuse(0.0f);


				for (auto light: lights) {
					glm::vec3 vectorToLight = light->position - payload.position;
					double distanceToLight = glm::l2Norm(vectorToLight);
					vectorToLight = glm::normalize(vectorToLight);

					glm::vec3 delta;
					if (glm::dot(payload.normal, vectorToLight) > 0) {
						delta = payload.normal * EPSILON;
					} else {
						delta = -payload.normal * EPSILON;
					}

					Ray shadowRay = Ray(payload.position + delta, vectorToLight);
					
					Intersection collision = traverse(shadowRay);
					// if no collision blocking the shadowRay, we add this light
					if (!collision.intersects || collision.t > distanceToLight) {
						// std::cout << "light: " << glm::to_string(light->colour) << std::endl;
						diffuse += fmax(glm::dot(payload.normal, 
							vectorToLight),0.0f) * light->colour;
						glm::vec3 h = glm::normalize(vectorToLight - ray.direction);
						specular += pow(fmax(glm::dot(payload.normal, h), 0.0f), 
							matInfo->shininess) * light->colour;
						
					}
					
				}

				return vec_min(ambient + diffuse * matInfo->kd + specular * matInfo->ks, white);
				break;
			}
		}
	}
	return glm::vec3(0.0f);
}

glm::vec3 A4_Scene::castRay(const Ray & ray, int depth) const {
	#ifdef PATH_TRACING
	#else
		return __RTCastRay(ray, depth);
	#endif
}

void A4_Canvas::render(const A4_Scene & scene) {
  	// Fill in raytracing code here...  

	// if main scene is not initalized, the do not render
	if (!scene.is_initialized()) return;

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
