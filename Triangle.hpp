#pragma once
#include "Object.hpp"

class Triangle:public Object
{
public:
	const glm::vec3 * v1;
	const glm::vec3 * v2;
	const glm::vec3 * v3;

	int i1;
	int i2;
	int i3;

	Triangle( const glm::vec3 * pv1, 
		const glm::vec3 * pv2, 
		const glm::vec3 * pv3,
		int i1,
		int i2,
		int i3)
		: v1( pv1 )
		, v2( pv2 )
		, v3( pv3 )
		, i1(i1)
		, i2(i2)
		, i3(i3)
	{}

	virtual ~Triangle() {};
	virtual Intersection intersect(const Ray &ray) const;
	virtual double sdf(const glm::vec3 &t) const;
	virtual AABB getAABB() const;
	virtual int construct() const;
};