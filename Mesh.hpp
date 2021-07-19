// Termm--Fall 2020

#pragma once

#include <vector>
#include <iosfwd>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "Primitive.hpp"
#include "BVH.hpp"
#include "Triangle.hpp"


// Use this #define to selectively compile your code to render the
// bounding boxes around your mesh objects. Uncomment this option
// to turn it on.
//#define RENDER_BOUNDING_VOLUMES

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh( const std::string& fname );
  virtual Intersection intersect(const Ray &ray) const;
  virtual double sdf(const glm::vec3 &t) const;
  virtual AABB getAABB() const;
  virtual ~Mesh();
private:
	std::vector<glm::vec3> m_vertices;
	std::vector<Triangle> m_faces;
	std::unique_ptr<BVH> bvh;

    friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};
