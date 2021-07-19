// Termm--Fall 2020

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {

public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	virtual ~GeometryNode();
	void setMaterial( Material *material );

	Material *m_material;
	Primitive *m_primitive;

	virtual Intersection intersect(const Ray &ray) const;

	virtual AABB getAABB() const;
};
