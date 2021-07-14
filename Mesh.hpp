// Termm--Fall 2020

#pragma once

#include <vector>
#include <iosfwd>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "BVH.hpp"


// Use this #define to selectively compile your code to render the
// bounding boxes around your mesh objects. Uncomment this option
// to turn it on.
//#define RENDER_BOUNDING_VOLUMES

class Triangle:public Object
{
public:
	const glm::vec3 * v1;
	const glm::vec3 * v2;
	const glm::vec3 * v3;

	Triangle( const glm::vec3 * pv1, 
		const glm::vec3 * pv2, 
		const glm::vec3 * pv3 )
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
	{}

	virtual ~Triangle() {};
	virtual Intersection intersect(const Ray &ray) const;
	virtual AABB getAABB() const;
};

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );
  virtual Intersection intersect(const Ray &ray) const;
  virtual AABB getAABB() const;
  virtual ~Mesh();
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	std::unique_ptr<BVH> bvh;

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
