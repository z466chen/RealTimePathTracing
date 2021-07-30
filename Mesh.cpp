// Termm--Fall 2020

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"
#include "defines.hpp"
#include "general.hpp"
#include "UboConstructor.hpp"

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{
	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;

	std::vector<object_reference> triangles;


	std::ifstream ifs( fname.c_str() );

	vertex_offset = UboConstructor::vert_arr.size();

	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );
			UboConstructor::vert_arr.push_back(UboVertex());
			UboConstructor::vert_arr.back().pos = glm::vec3(vx, vy, vz);
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle(
				&(*(m_vertices.begin() + (s1 - 1))), 
				&(*(m_vertices.begin() + (s2 - 1))), 
				&(*(m_vertices.begin() + (s3 - 1))), 
				s1-1, s2-1, s3-1));
		}
	}

	for (auto &trig: m_faces) {
		triangles.emplace_back(std::make_pair(&trig, std::make_pair(glm::mat4(1.0f), glm::mat4(1.0f))));
	}
	bvh = std::make_unique<BVH>(std::move(triangles), 1);
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

double Mesh::sdf(const glm::vec3 &t) const {
	return bvh->sdf(t);
} 

Intersection Mesh::intersect(const Ray &ray) const {
	auto result = bvh->intersect(ray);
	// if (result.intersects) {
	// 	result.obj = this;
	// }
	return result;
}

AABB Mesh::getAABB() const {
	return bvh->getAABB();
}

float Mesh::getArea(const glm::mat4 &t_matrix) const {
	int area  = 0.0f;
	for (auto &trig:m_faces) {
		area += trig.getArea(t_matrix);
	}
	return area;
}

int Mesh::construct(const glm::mat4 &t_matrix) const {
    int id = UboConstructor::obj_arr.size();
    UboConstructor::obj_arr.emplace_back(UboObject());
    
    AABB bbox = getAABB();
    UboConstructor::obj_arr[id].obj_aabb_1 = glm::vec2(bbox.lower_bound.x, bbox.lower_bound.y); 
    UboConstructor::obj_arr[id].obj_aabb_2 = glm::vec2(bbox.lower_bound.z, bbox.upper_bound.x); 
    UboConstructor::obj_arr[id].obj_aabb_3 = glm::vec2(bbox.upper_bound.y, bbox.upper_bound.z); 
    
	int bvh_id = bvh->construct(t_matrix);
    UboConstructor::obj_arr[id].obj_data_1 = glm::vec2(bvh_id, vertex_offset);
    UboConstructor::obj_arr[id].obj_type = (int)UboPrimitiveType::MESH;
    return id;
}

Mesh::~Mesh() {}