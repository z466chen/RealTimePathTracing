#include "Triangle.hpp"
#include "general.hpp"
#include "UboConstructor.hpp"

Intersection Triangle::intersect(const Ray &ray) const {
	Intersection result;

	glm::vec3 s = ray.origin - *v1;
	glm::vec3 e1 = *v2 - *v1;
	glm::vec3 e2 = *v3 - *v1;

	double divident = glm::dot(glm::cross(ray.direction, e2), e1);
	if (divident < EPSILON) {
		return result;
	}

	auto qvec = glm::cross(s, e1);

	double t0 = glm::dot(qvec, e2)/divident;
	double b1 = glm::dot(glm::cross(ray.direction, e2), s)/divident;
	double b2 = glm::dot(qvec, ray.direction)/divident;

	if (t0 > 0 && b1 >= 0 && b2 >= 0 && 1 - b1 - b2 >= 0) {
		// std::cout << "found: " << t0 << std::endl;
		
		result.intersects = true;
		result.t = t0;
		result.normal = glm::normalize(glm::cross(e1,e2));
		result.position = (float)result.t * ray.direction + ray.origin;
	}
	return result;
}

double Triangle::sdf(const glm::vec3 & t) const {
  glm::vec3 ba = *v2 - *v1; 
  glm::vec3 pa = t - *v1;
  glm::vec3 cb = *v3 - *v2; 
  glm::vec3 pb = t - *v2;
  glm::vec3 ac = *v1 - *v3; 
  glm::vec3 pc = t - *v3;
  glm::vec3 nor = glm::cross( ba, ac );

  return sqrt(
    (glm::sign(dot(cross(ba,nor),pa)) +
     glm::sign(dot(cross(cb,nor),pb)) +
     glm::sign(dot(cross(ac,nor),pc))<2.0)?
     fmin(fmin(
     dot2(ba*(float) glm::clamp(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
     dot2(cb*(float) glm::clamp(dot(cb,pb)/dot2(cb),0.0,1.0)-pb)),
     dot2(ac*(float) glm::clamp(dot(ac,pc)/dot2(ac),0.0,1.0)-pc))
     :
     dot(nor,pa)*dot(nor,pa)/dot2(nor) );	
}

AABB Triangle::getAABB() const {
	AABB result;
	result.lower_bound = vec_min(*v1, vec_min(*v2, *v3));
	result.upper_bound = vec_max(*v1, vec_max(*v2, *v3));
	return result;
}

int Triangle::construct() const {
	int id = UboConstructor::elem_arr.size();
	UboConstructor::elem_arr.emplace_back(UboElement());
	auto & ubo_elem = UboConstructor::elem_arr.back();
	ubo_elem.v1 = i1;
	ubo_elem.v2 = i2;
	ubo_elem.v3 = i3;
	return id;
}