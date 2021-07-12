// Termm--Fall 2020

#include <glm/ext.hpp>
#include "A4.hpp"
#include "PhongMaterial.hpp"
#include <queue>
#include <utility>

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

glm::vec3 A4_Scene::__RTCastRay(const Ray &ray,int depth, const glm::vec3 &background_color) const {
	if (depth > maxDepth) return background_color; 
	Intersection payload = traverse(ray);
	
	if (payload.intersects) {
		const PhongMaterial *material = 
			static_cast<PhongMaterial *>(payload.material);
			
		switch(payload.material->type) {
			case MaterialType::SPECULAR: {
 				double cos_i = glm::dot(-ray.direction, payload.normal);

				double theta_t;
				double kr;
				double n1 = 1.0f;
				double n2 = material->ior;
				if (cos_i < 0) {
					std::swap(n1, n2);
				}
				cos_i = fabs(cos_i);
				double theta_i = acos(cos_i);

				material->snell(theta_i, theta_t, n1, n2);
				material->fresnel(theta_i, theta_t, kr, n1, n2);

				glm::vec3 reflectDir = glm::normalize(2.0f * cos_i * payload.normal + ray.direction);
				glm::vec3 refractDir = glm::normalize(((n1/n2) * cos_i - cos(theta_t)) * payload.normal + 
					(n1/n2) * ray.direction);

				Ray reflectionRay = Ray(payload.position + 
					((glm::dot(payload.normal, reflectDir) > 0)? payload.normal*EPSILON: 
					-payload.normal*EPSILON), reflectDir);
			
				Ray refractionRay = Ray(payload.position + 
					((glm::dot(payload.normal, refractDir) > 0)? payload.normal*EPSILON: 
					-payload.normal*EPSILON), refractDir);

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
					}

				}

				return  ambient * material->m_kd + material->m_ks * (kr * castRay(reflectionRay,depth+1, background_color)+ 
					(1 - kr) * castRay(refractionRay,depth+1, background_color));
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
							material->m_shininess) * light->colour;
					}

				}

				// std::cout << "ambient: " << glm::to_string(ambient) << 
				// 		" diffuse: " << glm::to_string(diffuse) << 
				// 		" specular: " << glm::to_string(specular) << std::endl;
				return  ambient * material->m_kd + diffuse * material->m_kd + specular * material->m_ks;
				break;
			}
		}
	}
	return background_color;
}

glm::vec3 A4_Scene::castRay(const Ray & ray, int depth, const glm::vec3 &background_color) const {
	#ifdef PATH_TRACING
	#else
		return __RTCastRay(ray, depth, background_color);
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

	double w_size = tan(glm::radians(scene.camera.getFov()));
	double h_ratio = w_size/h;
	double w_ratio = w_size/w;
	
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			
			glm::vec3 dir = glm::vec3(((x + 0.5f)- w*0.5f)*h_ratio, (h*0.5f - (y + 0.5f))*w_ratio, -1);
			
			Ray primaryRay = Ray(scene.camera.getCamEye(), dir);
			glm::vec3 backgound_color = glm::vec3(
				background_img(x, y, 0),
				background_img(x, y, 1),
				background_img(x, y, 2)
			);

			glm::vec3 color = scene.castRay(primaryRay, 0, backgound_color);
			// Red: 
			image(x, y, 0) = (double)color.r;
			// Green: 
			image(x, y, 1) = (double)color.g;
			// Blue: 
			image(x, y, 2) = (double)color.b;
		}
	}

}
