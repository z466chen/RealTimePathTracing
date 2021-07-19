#pragma once
#include "Object.hpp"

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
	virtual double sdf(const glm::vec3 &t) const;
	virtual AABB getAABB() const;
};