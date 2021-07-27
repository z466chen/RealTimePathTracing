// Termm--Fall 2020

#include <glm/ext.hpp>
#include "Scene.hpp"
#include "PhongMaterial.hpp"
#include <queue>
#include <utility>
#include "general.hpp"

Scene::Scene(SceneNode *root, 
		glm::vec3 &eye, glm::vec3 &view, glm::vec3 &up, double fov, 
		glm::vec3 &ambient, std::list<Light *> &lights): root{root}, 
		camera{(float)fov, eye,glm::normalize(view - eye),  glm::normalize(up)},
		ambient{ambient}, lights{lights}{
	bvh = std::make_unique<BVH>(root);
}

Intersection Scene::traverse(const Ray &ray) const {
	return bvh->intersect(ray);
}

glm::vec3 Scene::__RTCastRay(const Ray &ray,int depth) const {
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

glm::vec3 Scene::castRay(const Ray & ray, int depth) const {
	#ifdef PATH_TRACING
	#else
		return __RTCastRay(ray, depth);
	#endif
}