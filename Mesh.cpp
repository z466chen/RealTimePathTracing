// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::vector<Object *> triangles;


	std::ifstream ifs( fname.c_str() );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( &(*(m_vertices.begin() + (s1 - 1))), 
				&(*(m_vertices.begin() + (s2 - 1))), 
				&(*(m_vertices.begin() + (s3 - 1)))));
		}
	}

	for (auto &trig: m_faces) {
		triangles.emplace_back(&trig);
	}

	std::cout << triangles.size() << std::endl;
	bvh = std::make_unique<BVH>(std::move(triangles));
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}

Intersection Triangle::intersect(const Ray &ray) {
	Intersection result;

	glm::vec3 s = ray.origin - *v1;
	glm::vec3 e1 = *v2 - *v1;
	glm::vec3 e2 = *v3 - *v1;

	double divident = glm::dot(glm::cross(ray.direction, e2), e1);

	double t0 = glm::dot(glm::cross(s, e1), e2)/divident;
	double b1 = glm::dot(glm::cross(ray.direction, e2), s)/divident;
	double b2 = glm::dot(glm::cross(s, e1), ray.direction)/divident;

	if (t0 > 0 && b1 > 0 && b2 > 0 && 1 - b1 - b2 > 0) {
		// std::cout << "found: " << t0 << std::endl;
		
		result.intersects = true;
		result.t = t0;
		result.normal = glm::cross(e1,e2);
		result.position = result.t * ray.direction + ray.origin;
	}
	return result;
}

AABB Triangle::getAABB() const {
	AABB result;
	result.lower_bound = glm::min(*v1, *v2, *v3);
	result.upper_bound = glm::max(*v1, *v2, *v3);
	return result;
}

Intersection Mesh::intersect(const Ray &ray) {
	auto result = bvh->intersect(ray);
	if (result.intersects) {
		result.obj = this;
	}
	return result;

	// Intersection result;
	// result.t = std::numeric_limits<double>::max();
	// for (auto t: m_faces) {
	// 	glm::vec3 s = ray.origin - *t.v1;
	// 	glm::vec3 e1 = *t.v2 - *t.v1;
	// 	glm::vec3 e2 = *t.v3 - *t.v1;

	// 	double divident = glm::dot(glm::cross(ray.direction, e2), e1);

	// 	double t0 = glm::dot(glm::cross(s, e1), e2)/divident;
	// 	double b1 = glm::dot(glm::cross(ray.direction, e2), s)/divident;
	// 	double b2 = glm::dot(glm::cross(s, e1), ray.direction)/divident;

	// 	if (t0 > 0 && t0 < result.t 
	// 		&& b1 >= 0 && b2 >= 0 && 1 - b1 - b2 >= 0) {
	// 		// std::cout << "found: " << t0 << std::endl;
			
	// 		result.intersects = true;
	// 		result.t = t0;
	// 		result.normal = glm::cross(e1,e2);
	// 		result.obj = this;
	// 		result.position = result.t * ray.direction + ray.origin;
	// 	}
	// }
	return result;
}

AABB Mesh::getAABB() const {
	return bvh->getAABB();
}

Mesh::~Mesh() {}