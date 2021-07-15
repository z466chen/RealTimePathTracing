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

glm::vec3 A4_Scene::__RTCastRay(const Ray &ray,int depth, const glm::vec3 &background_color) const {
	glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
	if (depth > maxDepth) return vec_min(background_color, white); 
	Intersection payload = traverse(ray);
	

	if (payload.intersects) {

		const PhongMaterial *material = 
			static_cast<const PhongMaterial *>(payload.material);
			
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


				return vec_min(ambient + material->m_ks * (kr * castRay(reflectionRay,depth+1, background_color)+ 
					(1 - kr) * castRay(refractionRay,depth+1, background_color)), white);
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
				return vec_min(ambient + diffuse * material->m_kd + specular * material->m_ks, white);
				break;
			}
		}
	}
	return vec_min(background_color, white);
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

  	// std::cout << "F20: Calling A4_Render(\n" <<
	// 	  "\t" << *scene.root <<
    //       "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
    //       "\t" << "eye:  " << glm::to_string(scene.camera.getCamEye()) << std::endl <<
	// 	  "\t" << "view: " << glm::to_string(scene.camera.getCamView()) << std::endl <<
	// 	  "\t" << "up:   " << glm::to_string(scene.camera.getCamUp()) << std::endl <<
	// 	  "\t" << "fovy: " << scene.camera.getFov() << std::endl <<
    //       "\t" << "ambient: " << glm::to_string(scene.ambient) << std::endl <<
	// 	  "\t" << "lights{" << std::endl;

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

	int count = 0;
	for (uint y = h - 1; y > 0; --y) {
		for (uint x = w - 1; x > 0; --x) {

			#ifdef ADAPTIVE
				glm::vec3 backgound_color;
				if (x > 0 && y > 0) {
					backgound_color	= glm::vec3(
						image(x, y, 0),
						image(x, y, 1),
						image(x, y, 2)
					);
				} else {
					backgound_color = glm::vec3(0,0,0);
				}
				 
				
				glm::vec3 dir = glm::vec3(((x + 0.5)- w*0.5f)*h_ratio, (h*0.5f - (y + 0.5))*w_ratio, -1);
				Ray primaryRay = Ray(scene.camera.getCamEye(), dir);
				glm::vec3 color = scene.castRay(primaryRay, 0, backgound_color);

				std::vector<std::pair<int, int>> n_deltas;
				n_deltas = {{0,1}, {1,0}, {1,1}};
				bool hit = false;
				for (auto &delta: n_deltas) {
					int a = x+delta.first;
					int b = y+delta.second;

					if (a < w && b < h) {
						glm::vec3 neighbour_backgound_color;
						if (a > 0 && b > 0) {
							neighbour_backgound_color	= glm::vec3(
								image(x, y, 0),
								image(x, y, 1),
								image(x, y, 2)
							);
						} else {
							neighbour_backgound_color = glm::vec3(0,0,0);
						}

						glm::vec3 color{image(a,b,0),image(a,b,1),image(a,b,2)};
						
						if (glm::l2Norm(color - neighbour_backgound_color) > 
							adaptive_thresh) {
							
							hit = true;
						}
					}
				}

				image(x, y, 0) = (double)0.0f;
				image(x, y, 1) = (double)0.0f;
				image(x, y, 2) = (double)0.0f;
				if (!hit) {
					// Red: 
					image(x, y, 0) += (double)color.r;
					// Green: 
					image(x, y, 1) += (double)color.g;
					// Blue: 
					image(x, y, 2) += (double)color.b;
					count += 1;
					continue;
				}
			#else
				glm::vec3 backgound_color = glm::vec3(
					image(x, y, 0),
					image(x, y, 1),
					image(x, y, 2)
				);


				image(x, y, 0) = (double)0.0f;
				image(x, y, 1) = (double)0.0f;
				image(x, y, 2) = (double)0.0f;
			#endif

			std::vector<std::pair<glm::vec2, float>> selections;
			sampler->pickInPixel(image, x, y, selections);

			for (auto &s: selections) {
				glm::vec3 dir = glm::vec3(((x + s.first.x)- w*0.5f)*h_ratio, (h*0.5f - (y + s.first.y))*w_ratio, -1);
				Ray primaryRay = Ray(scene.camera.getCamEye(), dir);
				
				glm::vec3 color = scene.castRay(primaryRay, 0, backgound_color);
				// Red: 
				image(x, y, 0) += (double)color.r*s.second;
				// Green: 
				image(x, y, 1) += (double)color.g*s.second;
				// Blue: 
				image(x, y, 2) += (double)color.b*s.second;

				if (s_type != SamplerType::SS_QUINCUNX || s.second > 0.2 ) continue; 
				
				std::vector<std::pair<int, int>> deltas;

				if (s.first.x > 0.5 && s.first.y > 0.5) {
					deltas = {{0,1}, {1,0}, {1,1}};
				} else if (s.first.x > 0.5) {
					deltas = {{1,0}};
				} else if (s.first.y > 0.5) {
					deltas = {{0,1}};
				}

				for (auto &delta: deltas) {
					int a = x+delta.first;
					int b = y+delta.second;

					if (a < w && b < h) {
						// Red: 
						image(a, b, 0) += (double)color.r*s.second;
						// Green: 
						image(a, b, 1) += (double)color.g*s.second;
						// Blue: 
						image(a, b, 2) += (double)color.b*s.second;
					}
					
				}
				
			}
			
		}
	}
	std::cout << "count:" << count << std::endl;
}
